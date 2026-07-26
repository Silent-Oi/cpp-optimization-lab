#include "oscillator_batch.h"
#include "state.h"

#include <iostream>
#include <iomanip>

int main() {
    const double dt = 0.013;
    const int step = 1000;
    const int number = 1000000;
    const int seed = 12345;
    oscillator::OscillatorBatch oscillator_aos_batch =
        oscillator::make_OscillatorAoS_batch(number, dt, seed);
    oscillator::update_aos_batch(oscillator_aos_batch, step);

    oscillator::AoSResults result = oscillator::aos_batch_report(oscillator_aos_batch);
    

    std::cout << std::setprecision(10) << "seed: " << seed << '\n';
    std::cout << std::setprecision(10) << "N: " << result.N << '\n';
    std::cout << std::setprecision(10) << "step: " << step << '\n';
    std::cout << std::setprecision(10) << "state_checksum: " << result.state_checksum << '\n';
    std::cout << std::setprecision(10) << "max_abs_x: " << result.max_abs_x << '\n';
    std::cout << std::setprecision(10) << "max_abs_v: " << result.max_abs_v << '\n';
    std::cout << std::setprecision(10) << "finite: " << result.finite << '\n';
    std::cout << '\n';
}
