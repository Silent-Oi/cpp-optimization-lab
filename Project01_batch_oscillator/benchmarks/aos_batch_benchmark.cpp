#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

#include "oscillator_batch.h"
#include "state.h"

// AoS benchmark harness configuration.
// number 在第一次测量前翻倍，因此当前覆盖 N=256 ... 1,048,576，共 13 个 2 倍规模点。
// 固定 step 和 seed，使不同 N 使用相同演化条件；cycle 表示每个 N 的重复测量次数。
const int cycle = 7;
int number = 128;
const int number_power = 13;
const int step = 100;
constexpr int seed = 1234;
constexpr double dt = 0.123;
// - Release, single thread
// - timed region contains only update_aos_batch()
// - one update means advancing one oscillator by one step
// - primary metric: ns per oscillator-step update

namespace fs = std::filesystem;

namespace benchmark {

struct BenchResults {
    int current_cycle;
    double run_time_second;
    double update_oscillator_per_second;
    double update_nanosecond_per_oscillator_step;
    oscillator::BatchResults aos_batch_results;
};

static double calu_average_time(const std::span<double>& time_array) {
    if (time_array.empty()) {
        throw std::invalid_argument("time_array cannot be empty");
    }

    double total_time = 0.0;
    for (double& time : time_array) {
        total_time += time;
    }
    return total_time / static_cast<double>(time_array.size());
}

static void print_bench_results(const std::vector<benchmark::BenchResults>& bench_results) {
    for (auto& result : bench_results) {
        std::cout << std::setprecision(10) << "current_cycle: " << result.current_cycle << '\n';
        std::cout << std::setprecision(10) << "run_time_second: " << result.run_time_second << '\n';
        std::cout << std::setprecision(10)
                  << "update_oscillator_per_second: " << result.update_oscillator_per_second
                  << '\n';
        std::cout << std::setprecision(10) << "update_nanosecond_per_oscillator_step: "
                  << result.update_nanosecond_per_oscillator_step << '\n';

        std::cout << std::setprecision(10)
                  << "state_checksum: " << result.aos_batch_results.state_checksum << '\n';
        std::cout << std::setprecision(10) << "max_abs_x: " << result.aos_batch_results.max_abs_x
                  << '\n';
        std::cout << std::setprecision(10) << "max_abs_v: " << result.aos_batch_results.max_abs_v
                  << '\n';
        std::cout << std::setprecision(10) << "----------------------------" << '\n';
        std::cout << std::setprecision(10) << '\n';
    }
}

}  // namespace benchmark

int main() {
    // 始终写入工作结果文件；确认一次测量有效后，再将其另存为 *_baseline.csv 提交。
    // 这样普通试跑不会直接覆盖仓库中的冻结 baseline。
    const fs::path result_directory = fs::path(PROJECT01_SOURCE_DIR) / "benchmarks" / "results";
    fs::create_directories(result_directory);

    const fs::path csv_path = result_directory / "aos_benchmark.csv";
    std::ofstream csv(csv_path);

    if (!csv) {
        throw std::runtime_error("无法创建 CSV 文件: " + csv_path.string());
    }

    csv << "N,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "dt: " << dt << '\n';
    std::cout << std::setprecision(10) << '\n';

    // 计时batch更新
    for (int j = 0; j < number_power; ++j) {
        number = number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorBatch initial_oscillator_aos_batch =
            oscillator::make_oscillator_aos_batch(number, dt, seed);

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorBatch warmup_batch = initial_oscillator_aos_batch;
            oscillator::update_aos_batch(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorBatch working_oscillator_aos_batch(
            initial_oscillator_aos_batch.size());

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            std::copy(initial_oscillator_aos_batch.begin(), initial_oscillator_aos_batch.end(),
                      working_oscillator_aos_batch.begin());

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_aos_batch(working_oscillator_aos_batch, step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::aos_batch_report(working_oscillator_aos_batch);
            if (!current_batch_result.finite) {
                throw std::runtime_error("振子更新结果异常");
            }

            double nanosecond_per_oscillator_step = run_time_second * 1e9 / counts;

            ns_records[i] = nanosecond_per_oscillator_step;
            const benchmark::BenchResults current_bench_result = {
                .current_cycle = i,
                .run_time_second = run_time_second,
                .update_oscillator_per_second = counts / run_time_second,
                .update_nanosecond_per_oscillator_step = nanosecond_per_oscillator_step,
                .aos_batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].aos_batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << number << ',' << step << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << number << '\n';
        // 这里只报告一个 AoS batch 的有效载荷；harness 还会持有预热和工作副本。
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(number) * sizeof(oscillator::OscillatorAoS) << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}
