#include "oscillator_batch.h"

#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>
#include <cstdlib>

#include "state.h"
#include "underdamped_oscillator.h"

namespace oscillator {

OscillatorBatch make_OscillatorAoS_batch(int number, double dt, int seed) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    // 固定 seed；每个振子拥有独立状态和参数。
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> position_uniform(-1, 1);
    std::uniform_real_distribution<double> velocity_uniform(-1, 1);
    std::uniform_real_distribution<double> omega_uniform(0.5, 5);
    std::uniform_real_distribution<double> zeta_uniform(0.0, 0.9);
    OscillatorBatch aos_batch(number);
    for (int i = 0; i < number; ++i) {
        double omega = omega_uniform(gen);
        double zeta = zeta_uniform(gen);
        UnderdampedOscillator system(omega, zeta);
        // 演化系数只在初始化阶段计算一次。
        StepCoefficients step_coefficients = system.make_step_coefficients(dt);

        OscillatorAoS oscillator{.position = position_uniform(gen),
                                 .velocity = velocity_uniform(gen),
                                 .omega = omega,
                                 .zeta = zeta,
                                 .m00 = step_coefficients.m00,
                                 .m01 = step_coefficients.m01,
                                 .m10 = step_coefficients.m10,
                                 .m11 = step_coefficients.m11};
        aos_batch[i] = oscillator;
    }
    return aos_batch;
}


void update_aos_batch_step(OscillatorBatch& aos_batch) {
    for (auto& oscillator : aos_batch) {
        // 避免 velocity 错误地使用已经更新的 position。
        double position =
            oscillator.m00 * oscillator.position + oscillator.m01 * oscillator.velocity;
        double velocity =
            oscillator.m10 * oscillator.position + oscillator.m11 * oscillator.velocity;
        oscillator.position = position;
        oscillator.velocity = velocity;
    }
}

void update_aos_batch(OscillatorBatch& aos_batch, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }
    // 热路径只连续遍历并调用预计算矩阵。
    for (int i = 0; i < step; ++i) {
        update_aos_batch_step(aos_batch);
    };
}

AoSResults aos_batch_report(OscillatorBatch& aos_batch_updated) {
    std::size_t N = aos_batch_updated.size();
    double state_checksum = 0.0;
    double max_abs_x = 0.0;
    double max_abs_v = 0.0;
    bool finite = 1;
    for (auto& oscillator : aos_batch_updated) {
        // checksum 压缩全部状态，max 值和 finite 则帮助识别发散或非有限结果。
        state_checksum += oscillator.position + 0.5 * oscillator.velocity;
        if (std::abs(oscillator.position) > max_abs_x) {
            max_abs_x = std::abs(oscillator.position);
        }
        if (std::abs(oscillator.velocity) > max_abs_v) {
            max_abs_v = std::abs(oscillator.velocity);
        }

        if (!std::isfinite(oscillator.position) || !std::isfinite(oscillator.velocity)) {
            finite = false;
            break;
        }
    }

    return {.N = N,
            .state_checksum = state_checksum,
            .max_abs_x = max_abs_x,
            .max_abs_v = max_abs_v,
            .finite = finite};
}
}  // namespace oscillator
