#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <algorithm>

#include "oscillator_batch.h"
#include "state.h"

namespace benchmark {

struct BenchResults {
    int current_cycle;
    int step;
    std::uint64_t counts;
    double run_time_second;
    double update_oscillator_per_second;
    double update_nanosecond_per_oscillator_step;
    oscillator::AoSResults aos_batch_results;
};

}  // namespace benchmark

int main() {
    // 初始化参数
    const int cycle = 7;
    constexpr int number = 1'000'000;
    std::vector<int> numbers = {256, 512, 768, 1'000, 10'000, 100'000, 1'000'000};
    std::vector<int> steps = {1'000, 1'000, 1'000, 1'000, 1'000, 1'000, 1'000};
    if (numbers.size() != steps.size())
    {
        throw std::invalid_argument("振子数量维度和计算布维度不一致");
    }
    const int numbers_count = static_cast<int>(numbers.size());
    constexpr int step = 1'000;
    constexpr int seed = 1234;
    constexpr double dt = 0.123;

    const int all_cycle = cycle * numbers_count;
    
    // 计时batch更新
    std::vector<benchmark::BenchResults> all_bench_results(all_cycle);
    for (int j = 0; j < numbers_count; ++j) {
        const oscillator::OscillatorBatch initial_oscillator_aos_batch =
            oscillator::make_OscillatorAoS_batch(numbers[j], dt, seed);

        // 预热
        oscillator::OscillatorBatch warmup_batch = initial_oscillator_aos_batch;
        oscillator::update_aos_batch(warmup_batch, steps[j]);

        for (int i = 0; i < cycle; ++i) {
            oscillator::OscillatorBatch working_oscillator_aos_batch = initial_oscillator_aos_batch;
            const auto start = std::chrono::steady_clock::now();
            oscillator::update_aos_batch(working_oscillator_aos_batch, steps[j]);
            const auto end = std::chrono::steady_clock::now();
            const auto run_time_second = std::chrono::duration<double>(end - start).count();

            // 统计结果
            oscillator::AoSResults current_batch_result =
                oscillator::aos_batch_report(working_oscillator_aos_batch);
            if (!current_batch_result.finite) {
                throw std::runtime_error("振子更新结果异常");
            }
            std::uint64_t counts = static_cast<std::uint64_t>(current_batch_result.N) *
                                   static_cast<std::uint64_t>(steps[j]);

            const benchmark::BenchResults current_bench_results = {
                .current_cycle = i,
                .step = steps[j],
                .counts = counts,
                .run_time_second = run_time_second,
                .update_oscillator_per_second = counts / run_time_second,
                .update_nanosecond_per_oscillator_step = run_time_second * 1e9 / counts,
                .aos_batch_results = current_batch_result,
            };

            // 收集结果
            int first_result_inde = j * cycle;
            int current_result_index = i + j * cycle;
            all_bench_results[current_result_index] = current_bench_results;
            if (current_batch_result != all_bench_results[first_result_inde].aos_batch_results) {
                throw std::runtime_error("振子更新不一致异常");
            }
        }
    }
    // 输出报告
    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "dt: " << dt << '\n';
    std::cout << std::setprecision(10) << '\n';

    for (int j = 0; j < numbers_count; ++j) {
        int current_index = j * cycle;
        std::cout << std::setprecision(10) << "########################################" << '\n';
        std::cout << std::setprecision(10) << "N: " << numbers[j] << '\n';
        std::cout << std::setprecision(10) << "size of input: " << numbers[j] * 64 << '\n';
        std::cout << std::setprecision(10) << "step: " << steps[j] << '\n';
        std::cout << std::setprecision(10) << "counts: " << all_bench_results[current_index].counts
                  << '\n';
        std::cout << std::setprecision(10) << '\n';
        std::array<double, cycle> samples;

        for (int i = 0; i < cycle; ++i) {
            double run_time_second = all_bench_results[current_index].run_time_second;

            samples[i] = run_time_second;
            
            std::cout << std::setprecision(10)
                      << "current_cycle: " << all_bench_results[current_index].current_cycle
                      << '\n';
            std::cout << std::setprecision(10) << "run_time_second: " << run_time_second << '\n';
            std::cout << std::setprecision(10) << "update_oscillator_per_second: "
                      << all_bench_results[current_index].update_oscillator_per_second << '\n';
            std::cout << std::setprecision(10) << "update_nanosecond_per_oscillator_step: "
                      << all_bench_results[current_index].update_nanosecond_per_oscillator_step
                      << '\n';
 
            std::cout << std::setprecision(10) << "state_checksum: "
                      << all_bench_results[current_index].aos_batch_results.state_checksum
                      << '\n';
            std::cout << std::setprecision(10) << "max_abs_x: "
                      << all_bench_results[current_index].aos_batch_results.max_abs_x << '\n';
            std::cout << std::setprecision(10) << "max_abs_v: "
                      << all_bench_results[current_index].aos_batch_results.max_abs_v << '\n';
                      
            std::cout << std::setprecision(10) << "----------------------------" << '\n';
            std::cout << std::setprecision(10) << '\n';
            current_index += 1;
        }

        std::sort(samples.begin(), samples.end());
        const double median = samples[cycle / 2];
        const double min_value = samples.front();
        const double max_value = samples.back();
        const double relative_range = (max_value - min_value) / median;

        std::cout << std::setprecision(10) << "median: " << median << '\n';
        std::cout << std::setprecision(10) << "max_cost: " << max_value << '\n';
        std::cout << std::setprecision(10) << "min_cost: " << min_value << '\n';
        std::cout << std::setprecision(10) << "relative_range: " << relative_range << '\n';
        std::cout << std::setprecision(10) << '\n';
    }
}
