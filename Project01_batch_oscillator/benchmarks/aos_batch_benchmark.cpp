#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>
#include <stdexcept>

#include "oscillator_batch.h"
#include "state.h"


struct BenchResults {
    int current_cycle;
    std::uint64_t counts;
    double run_time_second;
    double update_oscillator_per_second;
    double update_nanosecond_per_oscillator_step;
    oscillator::AoSResults aos_batch_results;
};


int main() {
    // 初始化参数
    constexpr int cycle = 7;
    constexpr int number = 1'000'000;
    constexpr int step = 1'000;
    constexpr int seed = 1234;
    constexpr double dt = 0.123;
    constexpr std::uint64_t counts = step * number;

    const oscillator::OscillatorBatch initial_oscillator_aos_batch =
        oscillator::make_OscillatorAoS_batch(number, dt, seed);

    // 预热
    oscillator::OscillatorBatch warmup_batch = initial_oscillator_aos_batch;
    oscillator::update_aos_batch(warmup_batch, step);

    // 计时batch更新
    std::vector<BenchResults> all_bench_results(cycle);
    for (int i = 0; i < cycle; ++i) {
        oscillator::OscillatorBatch benchmark_oscillator_aos_batch = initial_oscillator_aos_batch;
        const auto start = std::chrono::steady_clock::now();
        oscillator::update_aos_batch(benchmark_oscillator_aos_batch, step);
        const auto end = std::chrono::steady_clock::now();
        const auto run_time_second = std::chrono::duration<double>(end - start).count();

        // 统计结果
        oscillator::AoSResults current_batch_result =
            oscillator::aos_batch_report(benchmark_oscillator_aos_batch);
        if (!current_batch_result.finite)
        {
            throw std::runtime_error("振子更新结果异常");
        }
        const BenchResults current_bench_results = {
            .current_cycle = i,
            .counts = counts,
            .run_time_second = run_time_second,
            .update_oscillator_per_second = counts / run_time_second,
            .update_nanosecond_per_oscillator_step = run_time_second * 1e9 / counts,
            .aos_batch_results = current_batch_result,
        };

        all_bench_results[i] = current_bench_results;

        if (current_batch_result != all_bench_results[0].aos_batch_results)
        {
            throw std::runtime_error("振子更新不一致异常");
        }
    }
    // 输出报告
    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "N: " << number << '\n';
    std::cout << std::setprecision(10) << "step: " << step << '\n';
    std::cout << std::setprecision(10) << "counts: " << counts << '\n';
    std::cout << std::setprecision(10) << "dt: " << dt << '\n';
    std::cout << std::setprecision(10) << '\n';

    for (int i = 0; i < cycle; ++i) {
        std::cout << std::setprecision(10)
                  << "current_cycle: " << all_bench_results[i].current_cycle << '\n';
        std::cout << std::setprecision(10)
                  << "run_time_second: " << all_bench_results[i].run_time_second << '\n';
        std::cout << std::setprecision(10) << "update_oscillator_per_second: "
                  << all_bench_results[i].update_oscillator_per_second << '\n';
        std::cout << std::setprecision(10) << "update_nanosecond_per_oscillator_step: "
                  << all_bench_results[i].update_nanosecond_per_oscillator_step << '\n';
        std::cout << std::setprecision(10) << '\n';

        std::cout << std::setprecision(10)
                  << "state_checksum: " << all_bench_results[i].aos_batch_results.state_checksum
                  << '\n';
        std::cout << std::setprecision(10)
                  << "max_abs_x: " << all_bench_results[i].aos_batch_results.max_abs_x << '\n';
        std::cout << std::setprecision(10)
                  << "max_abs_v: " << all_bench_results[i].aos_batch_results.max_abs_v << '\n';
        std::cout << std::setprecision(10)
                  << "finite: " << all_bench_results[i].aos_batch_results.finite << '\n';
        std::cout << std::setprecision(10) << "----------------------------" << '\n';
        std::cout << std::setprecision(10) << '\n';
    }
}
