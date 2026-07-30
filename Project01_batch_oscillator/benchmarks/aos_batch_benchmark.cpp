#include <chrono>
#include <iostream>
#include <iomanip>
#include "state.h"
#include "oscillator_batch.h"

int main() {
    // 初始化参数
    constexpr int number = 1'000'000;
    constexpr int step = 1'000;
    constexpr int seed = 1234;
    constexpr double dt = 0.123;
    constexpr int counts = step * number;

    const oscillator::OscillatorBatch initial_oscillator_aos_batch =
        oscillator::make_OscillatorAoS_batch(number, dt, seed);

    // 预热
    oscillator::OscillatorBatch warmup_batch = initial_oscillator_aos_batch;
    oscillator::update_aos_batch(warmup_batch, step);

    // 计时batch更新
    oscillator::OscillatorBatch benchmark_oscillator_aos_batch = initial_oscillator_aos_batch;
    const auto start = std::chrono::steady_clock::now();
    oscillator::update_aos_batch(benchmark_oscillator_aos_batch, step);
    const auto end = std::chrono::steady_clock::now();

    const auto run_time = std::chrono::duration<double>(end - start).count();

    // 统计结果
    oscillator::AoSResults result = oscillator::aos_batch_report(benchmark_oscillator_aos_batch);

    // 输出报告
    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "N: " << result.N << '\n';
    std::cout << std::setprecision(10) << "step: " << step << '\n';
    std::cout << std::setprecision(10) << "counts: " << counts << '\n';
    std::cout << std::setprecision(10) << "runtime: " << run_time << '\n';
    std::cout << std::setprecision(10) << "state_checksum: " << result.state_checksum << '\n';
    std::cout << std::setprecision(10) << "max_abs_x: " << result.max_abs_x << '\n';
    std::cout << std::setprecision(10) << "max_abs_v: " << result.max_abs_v << '\n';
    std::cout << std::setprecision(10) << "finite: " << result.finite << '\n';
    std::cout << '\n';


}
