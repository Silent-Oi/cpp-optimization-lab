#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "oscillator_batch.h"
#include "state.h"

namespace fs = std::filesystem;

// ============================================================================
// 实验基础数据
// ============================================================================

namespace {
const int cycle = 7;
int number = 64;
const int power = 16;
const int step = 16;
constexpr int seed = 1234;
constexpr double dt = 0.123;
}  // namespace

// ============================================================================
// benchmark 相关函数与结构
// ============================================================================

namespace benchmark {

struct BenchResults {
    int current_cycle;
    double run_time_second;
    double update_oscillator_per_second;
    double update_nanosecond_per_oscillator_step;
    oscillator::BatchResults batch_results;
};

static std::string make_run_timestamp() {
    using namespace std::chrono;

    const auto now = floor<seconds>(system_clock::now());
    const zoned_time local_time{current_zone(), now};

    return std::format("{:%Y%m%d_%H%M%S}", local_time);
}

static std::ofstream create_result_csv(const std::string& data, const std::string& experiment_name,
                                       const std::string& filename) {
    const fs::path result_directory =
        fs::path(PROJECT01_SOURCE_DIR) / "benchmarks" / experiment_name / "results";
    fs::create_directories(result_directory);

    std::string filename_data = filename + data;

    const fs::path csv_path = result_directory / filename_data;
    std::ofstream csv(csv_path);

    if (!csv) {
        throw std::runtime_error("无法创建 CSV 文件: " + csv_path.string());
    }

    return csv;
}

static double calu_average_time(const std::span<double>& time_array) {
    if (time_array.empty()) {
        throw std::invalid_argument("time_array cannot be empty");
    }

    double total_time = 0.0;
    for (double& time : time_array) {
        total_time += time;
    }
    return total_time / static_cast<double>(time_array.size());
};

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
                  << "state_checksum: " << result.batch_results.state_checksum << '\n';
        std::cout << std::setprecision(10) << "max_abs_x: " << result.batch_results.max_abs_x
                  << '\n';
        std::cout << std::setprecision(10) << "max_abs_v: " << result.batch_results.max_abs_v
                  << '\n';
        std::cout << std::setprecision(10) << "----------------------------" << '\n';
        std::cout << std::setprecision(10) << '\n';
    }
}
}  // namespace benchmark

// ============================================================================
// 实验一 同规模下AOS与SOA之间的对比
// ============================================================================

namespace layout_experiment {

// AOS benchmark
// ============================================================================

static void benchmark_aos(const std::string& data, const std::string& experiment_name,
                          const std::string& filename, int initial_number) {
    // ---- 创建AOS实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK AOS     *********************" << '\n';

    // ---- 进入AOS更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorAoSBatch initial_oscillator_aos_batch =
            oscillator::make_oscillator_aos_batch(initial_number, dt, seed);

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorAoSBatch warmup_batch = initial_oscillator_aos_batch;
            oscillator::update_aos_batch(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorAoSBatch working_oscillator_aos_batch(
            initial_oscillator_aos_batch.size());

        // ---- 进入AOS更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << step << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        // 这里只报告一个 AoS batch 的有效载荷；harness 还会持有预热和工作副本。
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeof(oscillator::OscillatorAoS)
                  << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

// SOA benchmark
// ============================================================================

static void benchmark_soa(const std::string& data, const std::string& experiment_name,
                          const std::string& filename, int initial_number) {
    // ---- 创建SOA实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK SOA     *********************" << '\n';

    // ---- 进入AOS更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorSoABatch_no_termination initial_oscillator_soa_batch =
            oscillator::make_oscillator_soa_batch_no_termination(initial_number, dt, seed);

        const std::size_t N = initial_oscillator_soa_batch.omega.size();

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorSoABatch_no_termination warmup_batch =
                initial_oscillator_soa_batch;
            oscillator::update_soa_batch_no_termination(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorSoABatch_no_termination working_oscillator_soa_batch{
            .position = std::vector<double>(N),
            .velocity = std::vector<double>(N),
            .m00 = std::vector<double>(N),
            .m01 = std::vector<double>(N),
            .m10 = std::vector<double>(N),
            .m11 = std::vector<double>(N),
            .omega = std::vector<double>(N),
            .zeta = std::vector<double>(N),
        };

        // ---- 进入AOS更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            std::copy(initial_oscillator_soa_batch.position.begin(),
                      initial_oscillator_soa_batch.position.end(),
                      working_oscillator_soa_batch.position.begin());
            std::copy(initial_oscillator_soa_batch.velocity.begin(),
                      initial_oscillator_soa_batch.velocity.end(),
                      working_oscillator_soa_batch.velocity.begin());
            std::copy(initial_oscillator_soa_batch.m00.begin(),
                      initial_oscillator_soa_batch.m00.end(),
                      working_oscillator_soa_batch.m00.begin());
            std::copy(initial_oscillator_soa_batch.m01.begin(),
                      initial_oscillator_soa_batch.m01.end(),
                      working_oscillator_soa_batch.m01.begin());
            std::copy(initial_oscillator_soa_batch.m10.begin(),
                      initial_oscillator_soa_batch.m10.end(),
                      working_oscillator_soa_batch.m10.begin());
            std::copy(initial_oscillator_soa_batch.m11.begin(),
                      initial_oscillator_soa_batch.m11.end(),
                      working_oscillator_soa_batch.m11.begin());
            std::copy(initial_oscillator_soa_batch.omega.begin(),
                      initial_oscillator_soa_batch.omega.end(),
                      working_oscillator_soa_batch.omega.begin());
            std::copy(initial_oscillator_soa_batch.zeta.begin(),
                      initial_oscillator_soa_batch.zeta.end(),
                      working_oscillator_soa_batch.zeta.begin());

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_soa_batch_no_termination(working_oscillator_soa_batch, step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::soa_batch_report(working_oscillator_soa_batch);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << step << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeof(oscillator::OscillatorAoS)
                  << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

}  // namespace layout_experiment

// ============================================================================
// 实验二 数据结构大小（带额外字段）对计算性能的影响
// ============================================================================

namespace aos_size_experiment {

// 对齐数据结构 benchmark
// ============================================================================

static void benchmark_aos(const std::string& data, const std::string& experiment_name,
                          const std::string& filename, int initial_number) {
    // ---- 创建对齐数据实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,sizeof,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK AOS     *********************" << '\n';

    auto sizeofoscillator = sizeof(oscillator::OscillatorAoS);

    // ---- 进入AOS更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorAoSBatch initial_oscillator_aos_batch =
            oscillator::make_oscillator_aos_batch(initial_number, dt, seed);

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorAoSBatch warmup_batch = initial_oscillator_aos_batch;
            oscillator::update_aos_batch(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorAoSBatch working_oscillator_aos_batch(
            initial_oscillator_aos_batch.size());

        // ---- 进入AOS更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << sizeofoscillator << ',' << step << ',' << average_ns << ','
            << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << "size of struct: " << sizeofoscillator << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        // 这里只报告一个 AoS batch 的有效载荷；harness 还会持有预热和工作副本。
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeofoscillator << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

// 带额外字段数据结构 benchmark
// ============================================================================

static void benchmark_aos_with_payload(const std::string& data, const std::string& experiment_name,
                                       const std::string& filename, int initial_number) {
    // ---- 创建带额外字段数据实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,sizeof,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK AOS With Payload     *********************"
              << '\n';

    // ---- 进入带额外字段AOS更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    auto sizeofoscillator = sizeof(oscillator::OscillatorAoSWithPayload);

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorAoSBatchWithPayload initial_oscillator_aos_batch =
            oscillator::make_oscillator_aos_batch_with_payload(initial_number, dt, seed);

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorAoSBatchWithPayload warmup_batch = initial_oscillator_aos_batch;
            oscillator::update_aos_batch_with_payload(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorAoSBatchWithPayload working_oscillator_aos_batch(
            initial_oscillator_aos_batch.size());

        // ---- 进入带额外字段AOS更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            std::copy(initial_oscillator_aos_batch.begin(), initial_oscillator_aos_batch.end(),
                      working_oscillator_aos_batch.begin());

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_aos_batch_with_payload(working_oscillator_aos_batch, step);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << sizeofoscillator << ',' << step << ',' << average_ns << ','
            << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << "size of struct: " << sizeofoscillator << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        // 这里只报告一个 AoS batch 的有效载荷；harness 还会持有预热和工作副本。
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeofoscillator << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}
}  // namespace aos_size_experiment

// ============================================================================
// 实验三 终止计算的收益
// ============================================================================

namespace termination_experiment {

// 有终止SOA benchmark
// ============================================================================

static void benchmark_soa(const std::string& data, const std::string& experiment_name,
                          const std::string& filename, int initial_step) {
    // ---- 创建无终止SOA实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "Steps,N,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK SOA     *********************" << '\n';

    // ---- 进入有终止SOA更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_step = initial_step * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(number) * static_cast<std::uint64_t>(initial_step);

        const oscillator::OscillatorSoABatch initial_oscillator_soa_batch =
            oscillator::make_oscillator_soa_batch(number, dt, seed);

        const std::size_t N = initial_oscillator_soa_batch.omega.size();

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorSoABatch warmup_batch = initial_oscillator_soa_batch;
            oscillator::update_soa_batch(warmup_batch, initial_step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorSoABatch working_oscillator_soa_batch{
            .position = std::vector<double>(N),
            .velocity = std::vector<double>(N),
            .m00 = std::vector<double>(N),
            .m01 = std::vector<double>(N),
            .m10 = std::vector<double>(N),
            .m11 = std::vector<double>(N),
            .omega = std::vector<double>(N),
            .zeta = std::vector<double>(N),
            .active_indices = std::vector<int>(N),
        };

        // ---- 进入有终止SOA更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            working_oscillator_soa_batch = initial_oscillator_soa_batch;

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_soa_batch(working_oscillator_soa_batch, initial_step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::soa_batch_report(working_oscillator_soa_batch);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_step << ',' << number << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "Steps: " << initial_step << '\n';
        std::cout << std::setprecision(10) << "N: " << number << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

// 无终止SOA benchmark
// ============================================================================

static void benchmark_soa_no_termination(const std::string& data,
                                         const std::string& experiment_name,
                                         const std::string& filename, int initial_step) {
    // ---- 创建无终止SOA实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "Steps,N,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK SOA     *********************" << '\n';

    // ---- 进入无终止SOA更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_step = initial_step * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(number) * static_cast<std::uint64_t>(initial_step);

        const oscillator::OscillatorSoABatch_no_termination initial_oscillator_soa_batch =
            oscillator::make_oscillator_soa_batch_no_termination(number, dt, seed);

        const std::size_t N = initial_oscillator_soa_batch.omega.size();

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorSoABatch_no_termination warmup_batch =
                initial_oscillator_soa_batch;
            oscillator::update_soa_batch_no_termination(warmup_batch, initial_step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorSoABatch_no_termination working_oscillator_soa_batch{
            .position = std::vector<double>(N),
            .velocity = std::vector<double>(N),
            .m00 = std::vector<double>(N),
            .m01 = std::vector<double>(N),
            .m10 = std::vector<double>(N),
            .m11 = std::vector<double>(N),
            .omega = std::vector<double>(N),
            .zeta = std::vector<double>(N),
        };

        // ---- 进入无终止SOA更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            working_oscillator_soa_batch = initial_oscillator_soa_batch;

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_soa_batch_no_termination(working_oscillator_soa_batch, initial_step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::soa_batch_report(working_oscillator_soa_batch);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_step << ',' << number << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "Steps: " << initial_step << '\n';
        std::cout << std::setprecision(10) << "N: " << number << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

}  // namespace termination_experiment

// ============================================================================
// 实验四 向量化收益
// ============================================================================

namespace vectorization_experiment {

// 向量化无终止SOA benchmark
// ============================================================================

static void benchmark_soa(const std::string& data, const std::string& experiment_name,
                          const std::string& filename, int initial_number) {
    // ---- 创建向量化无终止SOA实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK SOA     *********************" << '\n';

    // ---- 进入向量化无终止SOA更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorSoABatch_no_termination initial_oscillator_soa_batch =
            oscillator::make_oscillator_soa_batch_no_termination(initial_number, dt, seed);

        const std::size_t N = initial_oscillator_soa_batch.omega.size();

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorSoABatch_no_termination warmup_batch =
                initial_oscillator_soa_batch;
            oscillator::update_soa_batch_no_termination(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorSoABatch_no_termination working_oscillator_soa_batch{
            .position = std::vector<double>(N),
            .velocity = std::vector<double>(N),
            .m00 = std::vector<double>(N),
            .m01 = std::vector<double>(N),
            .m10 = std::vector<double>(N),
            .m11 = std::vector<double>(N),
            .omega = std::vector<double>(N),
            .zeta = std::vector<double>(N),
        };

        // ---- 进入向量化无终止SOA更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            working_oscillator_soa_batch = initial_oscillator_soa_batch;

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_soa_batch_no_termination(working_oscillator_soa_batch, step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::soa_batch_report(working_oscillator_soa_batch);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << step << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeof(oscillator::OscillatorAoS)
                  << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

// 标量无终止SOA benchmark
// ============================================================================

static void benchmark_soa_scalar(const std::string& data, const std::string& experiment_name,
                                 const std::string& filename, int initial_number) {
    // ---- 创建标量无终止SOA实验结果CSV文件与数据记录形式
    // --------------------------------------------------------------

    std::ofstream csv = benchmark::create_result_csv(data, experiment_name, filename);
    csv << "N,steps,average_ns,median_ns\n";

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK SOA     *********************" << '\n';

    // ---- 进入标量无终止SOA更新计算，总共实验数量为 power
    // --------------------------------------------------------------

    for (int j = 0; j < power; ++j) {
        initial_number = initial_number * 2;
        std::uint64_t counts =
            static_cast<std::uint64_t>(initial_number) * static_cast<std::uint64_t>(step);

        const oscillator::OscillatorSoABatch_no_termination initial_oscillator_soa_batch =
            oscillator::make_oscillator_soa_batch_no_termination(initial_number, dt, seed);

        const std::size_t N = initial_oscillator_soa_batch.omega.size();

        {
            // 使用独立副本预热代码路径，避免改变后续各轮共享的初始状态；预热不计时。
            oscillator::OscillatorSoABatch_no_termination warmup_batch =
                initial_oscillator_soa_batch;
            oscillator::update_soa_batch_no_termination_scalar(warmup_batch, step);
        }

        std::array<double, cycle> ns_records;
        std::vector<benchmark::BenchResults> bench_results(cycle);

        oscillator::OscillatorSoABatch_no_termination working_oscillator_soa_batch{
            .position = std::vector<double>(N),
            .velocity = std::vector<double>(N),
            .m00 = std::vector<double>(N),
            .m01 = std::vector<double>(N),
            .m10 = std::vector<double>(N),
            .m11 = std::vector<double>(N),
            .omega = std::vector<double>(N),
            .zeta = std::vector<double>(N),
        };

        // ---- 进入标量无终止SOA更新计算，总共实验次数为 cycle
        // --------------------------------------------------------------

        for (int i = 0; i < cycle; ++i) {
            // 每轮从完全相同的输入开始；复制发生在计时区间之外。
            working_oscillator_soa_batch = initial_oscillator_soa_batch;

            // 计时区间只包含核心批量更新，不包含初始化、复制、校验和输出。
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_soa_batch_no_termination_scalar(working_oscillator_soa_batch, step);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 在计时后消费全部最终状态：既检查数值有效性，也保留可比较的结果摘要。
            oscillator::BatchResults current_batch_result =
                oscillator::soa_batch_report(working_oscillator_soa_batch);
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
                .batch_results = current_batch_result,
            };

            bench_results[i] = current_bench_result;

            // 第 0 轮作为参考；确定性输入应在所有重复测量中得到一致摘要。
            if (current_batch_result != bench_results[0].batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }

        // ---- 输出保存实验结果
        // --------------------------------------------------------------

        // CSV 保存当前 N 的算术平均值；逐轮原始数据仍打印到控制台便于观察抖动。
        std::sort(ns_records.begin(), ns_records.end());
        const double median_ns = ns_records[cycle / 2];
        double average_ns = benchmark::calu_average_time(ns_records);
        csv << initial_number << ',' << step << ',' << average_ns << ',' << median_ns << '\n';

        // 输出结果
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << initial_number << '\n';
        std::cout << std::setprecision(10) << "size of input: "
                  << static_cast<std::size_t>(initial_number) * sizeof(oscillator::OscillatorAoS)
                  << '\n';
        std::cout << std::setprecision(10) << "step: " << step << '\n';
        std::cout << std::setprecision(10) << "counts: " << counts << '\n';
        std::cout << std::setprecision(10) << '\n';
        benchmark::print_bench_results(bench_results);
    }
}

}  // namespace vectorization_experiment

int main() {
    auto data = benchmark::make_run_timestamp();

    // ---- 打印实验参数
    // --------------------------------------------------------------
    std::cout << std::setprecision(10) << "date: " << data << '\n';
    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "dt: " << dt << '\n';
    std::cout << std::setprecision(10) << "number_power: " << power << '\n';
    std::cout << std::setprecision(10) << '\n';

    /*
    // ============================================================================
    // 实验一 AOS vs SOA
    // ============================================================================

    std::string experiment_name = "layout_experiment";

    std::string filename_aos = "aos_benchmark.csv";
    std::string filename_soa = "soa_benchmark.csv";
    layout_experiment::benchmark_soa(data, experiment_name, filename_soa, number);
    layout_experiment::benchmark_aos(data, experiment_name, filename_aos, number);
    */

    /*
    // ============================================================================
    // 实验二 AOS vs AOS With Payload
    // ============================================================================

    std::string experiment_name = "aos_size_experiment";

    std::string filename_aos = "aos64_benchmark.csv";
    std::string filename_aos_with_payload = "aos72_benchmark.csv";
    aos_size_experiment::benchmark_aos(data, experiment_name, filename_aos, number);
    aos_size_experiment::benchmark_aos_with_payload(data, experiment_name,
    filename_aos_with_payload, number);
    */

    /*
    // ============================================================================
    // 实验三 终止对计算的影响
    // ============================================================================

    std::string experiment_name = "termination_experiment";

    std::string filename_soa = "soa_benchmark.csv";
    std::string filename_soa_no_termination = "soa_no_termination_benchmark.csv";
    termination_experiment::benchmark_soa(data, experiment_name, filename_soa, number);
    termination_experiment::benchmark_soa_no_termination(data, experiment_name,
                                                         filename_soa_no_termination, number);
    */

    // ============================================================================
    // 实验四 向量化对计算速度影响
    // ============================================================================

    std::string experiment_name = "vectorization_experiment";

    std::string filename_soa = "soa_benchmark.csv";
    std::string filename_soa_scalar = "soa_scalar_benchmark.csv";
    vectorization_experiment::benchmark_soa(data, experiment_name, filename_soa, number);
    vectorization_experiment::benchmark_soa_scalar(data, experiment_name, filename_soa_scalar,
                                                   number);
}
