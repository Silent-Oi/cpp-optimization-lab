#include "oscillator_batch.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numbers>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "state.h"
#include "underdamped_oscillator.h"

namespace {
double eps_energy = 1E-12;

static double system_energy(const double& omega, const double& position, const double& velocity) {
    return 0.5 * velocity * velocity + 0.5 * omega * omega * position * position;
}
}  // namespace

namespace oscillator {

// ============================================================================
// AOS振子相关
// ============================================================================

// ---- AOS振子创建 --------------------------------------------------------------

OscillatorAoSBatch make_oscillator_aos_batch(int number, double dt, int seed) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    // 固定 seed；每个振子拥有独立状态和参数。
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);
    std::uniform_real_distribution<double> omega_uniform(0.5, 5);
    std::uniform_real_distribution<double> zeta_uniform(0.0, 0.9);
    OscillatorAoSBatch aos_batch(number);
    for (int i = 0; i < number; ++i) {
        double omega = omega_uniform(gen);
        double zeta = zeta_uniform(gen);

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        UnderdampedOscillator system(omega, zeta);
        // 演化系数只在初始化阶段计算一次。
        StepCoefficients step_coefficients = system.make_step_coefficients(dt);

        OscillatorAoS oscillator{.position = r * cos(theta),
                                 .velocity = r * sin(theta),
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

// ---- AOS振子更新 --------------------------------------------------------------

void update_aos_batch_step(OscillatorAoSBatch& aos_batch) {
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

void update_aos_batch(OscillatorAoSBatch& aos_batch, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }
    // 热路径只连续遍历并调用预计算矩阵。
    for (int i = 0; i < step; ++i) {
        update_aos_batch_step(aos_batch);
    };
}

// ---- 非对齐 AOS振子创建
// --------------------------------------------------------------

OscillatorAoSBatchWithPayload make_oscillator_aos_batch_with_payload(int number, double dt,
                                                                     int seed) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    // 固定 seed；每个振子拥有独立状态和参数。
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);
    std::uniform_real_distribution<double> omega_uniform(0.5, 5);
    std::uniform_real_distribution<double> zeta_uniform(0.0, 0.9);
    OscillatorAoSBatchWithPayload aos_batch(number);
    for (int i = 0; i < number; ++i) {
        double omega = omega_uniform(gen);
        double zeta = zeta_uniform(gen);

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        UnderdampedOscillator system(omega, zeta);
        // 演化系数只在初始化阶段计算一次。
        StepCoefficients step_coefficients = system.make_step_coefficients(dt);

        OscillatorAoSWithPayload oscillator{.position = r * cos(theta),
                                            .velocity = r * sin(theta),
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

// ---- 非对齐 AOS振子更新
// --------------------------------------------------------------

void update_aos_batch_step_with_payload(OscillatorAoSBatchWithPayload& aos_batch) {
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

void update_aos_batch_with_payload(OscillatorAoSBatchWithPayload& aos_batch, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }
    // 热路径只连续遍历并调用预计算矩阵。
    for (int i = 0; i < step; ++i) {
        update_aos_batch_step_with_payload(aos_batch);
    };
}

// ---- AOS振子输出相关信息 --------------------------------------------------------------

BatchResults aos_batch_report(const OscillatorAoSBatch& aos_batch_updated) {
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

BatchResults aos_batch_report(const OscillatorAoSBatchWithPayload& aos_batch_updated) {
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

// ============================================================================
// SOA振子相关
// ============================================================================

// ---- SOA振子创建 --------------------------------------------------------------

OscillatorSoABatch make_oscillator_soa_batch(int number, double dt, int seed) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    const std::size_t count = static_cast<std::size_t>(number);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);
    std::uniform_real_distribution<double> omega_uniform(0.5, 5);
    std::uniform_real_distribution<double> zeta_uniform(0.0, 0.9);

    OscillatorSoABatch soa_batch = {
        .position = std::vector<double>(count),
        .velocity = std::vector<double>(count),
        .m00 = std::vector<double>(count),
        .m01 = std::vector<double>(count),
        .m10 = std::vector<double>(count),
        .m11 = std::vector<double>(count),
        .omega = std::vector<double>(count),
        .zeta = std::vector<double>(count),
        .active_indices = std::vector<int>(count),
    };

    for (int i = 0; i < number; ++i) {
        soa_batch.omega[i] = omega_uniform(gen);
        soa_batch.zeta[i] = zeta_uniform(gen);

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        soa_batch.position[i] = r * cos(theta);
        soa_batch.velocity[i] = r * sin(theta);

        UnderdampedOscillator system(soa_batch.omega[i], soa_batch.zeta[i]);
        StepCoefficients step_coefficients = system.make_step_coefficients(dt);
        soa_batch.m00[i] = step_coefficients.m00;
        soa_batch.m01[i] = step_coefficients.m01;
        soa_batch.m10[i] = step_coefficients.m10;
        soa_batch.m11[i] = step_coefficients.m11;

        soa_batch.active_indices[i] = i;
    }

    return soa_batch;
}

OscillatorSoABatch make_oscillator_soa_batch(int number, double dt, int seed, double omega,
                                             double zeta) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    const std::size_t count = static_cast<std::size_t>(number);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);

    OscillatorSoABatch soa_batch = {
        .position = std::vector<double>(count),
        .velocity = std::vector<double>(count),
        .m00 = std::vector<double>(count),
        .m01 = std::vector<double>(count),
        .m10 = std::vector<double>(count),
        .m11 = std::vector<double>(count),
        .omega = std::vector<double>(count),
        .zeta = std::vector<double>(count),
        .active_indices = std::vector<int>(count),
    };

    UnderdampedOscillator system(omega, zeta);
    StepCoefficients step_coefficients = system.make_step_coefficients(dt);

    for (int i = 0; i < number; ++i) {
        soa_batch.omega[i] = omega;
        soa_batch.zeta[i] = zeta;

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        soa_batch.position[i] = r * cos(theta);
        soa_batch.velocity[i] = r * sin(theta);

        soa_batch.m00[i] = step_coefficients.m00;
        soa_batch.m01[i] = step_coefficients.m01;
        soa_batch.m10[i] = step_coefficients.m10;
        soa_batch.m11[i] = step_coefficients.m11;

        soa_batch.active_indices[i] = i;
    }

    return soa_batch;
}

// ---- 带终止的SOA振子更新 --------------------------------------------------------------

void update_soa_batch_step(OscillatorSoABatch& soa_batch) {
    for (std::size_t i = 0; i < soa_batch.active_indices.size();) {
        int batch_index = soa_batch.active_indices[i];
        const double old_position = soa_batch.position[batch_index];
        const double old_velocity = soa_batch.velocity[batch_index];

        double energy = system_energy(soa_batch.omega[batch_index], old_position, old_velocity);

        if (energy < eps_energy) {
            soa_batch.active_indices[i] = std::move(soa_batch.active_indices.back());
            soa_batch.active_indices.pop_back();
            continue;
        }

        soa_batch.position[batch_index] =
            soa_batch.m00[batch_index] * old_position + soa_batch.m01[batch_index] * old_velocity;
        soa_batch.velocity[batch_index] =
            soa_batch.m10[batch_index] * old_position + soa_batch.m11[batch_index] * old_velocity;
        ++i;
    }
}

void update_soa_batch(OscillatorSoABatch& soa_batch, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    for (int i = 0; i < step; ++i) {
        if (soa_batch.active_indices.size() == 0) {
            break;
        }
        update_soa_batch_step(soa_batch);
    };
}

// ---- 无终止的SOA振子创建 --------------------------------------------------------------

OscillatorSoABatch_no_termination make_oscillator_soa_batch_no_termination(int number, double dt,
                                                                           int seed) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    const std::size_t count = static_cast<std::size_t>(number);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);
    std::uniform_real_distribution<double> omega_uniform(0.5, 5);
    std::uniform_real_distribution<double> zeta_uniform(0.0, 0.9);

    OscillatorSoABatch_no_termination soa_batch = {
        .position = std::vector<double>(count),
        .velocity = std::vector<double>(count),
        .m00 = std::vector<double>(count),
        .m01 = std::vector<double>(count),
        .m10 = std::vector<double>(count),
        .m11 = std::vector<double>(count),
        .omega = std::vector<double>(count),
        .zeta = std::vector<double>(count),
    };

    for (int i = 0; i < number; ++i) {
        soa_batch.omega[i] = omega_uniform(gen);
        soa_batch.zeta[i] = zeta_uniform(gen);

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        soa_batch.position[i] = r * cos(theta);
        soa_batch.velocity[i] = r * sin(theta);

        UnderdampedOscillator system(soa_batch.omega[i], soa_batch.zeta[i]);
        StepCoefficients step_coefficients = system.make_step_coefficients(dt);
        soa_batch.m00[i] = step_coefficients.m00;
        soa_batch.m01[i] = step_coefficients.m01;
        soa_batch.m10[i] = step_coefficients.m10;
        soa_batch.m11[i] = step_coefficients.m11;
    }

    return soa_batch;
}

OscillatorSoABatch_no_termination make_oscillator_soa_batch_no_termination(int number, double dt,
                                                                           int seed, double omega,
                                                                           double zeta) {
    if (number < 0) {
        throw std::invalid_argument("oscillator number must be non-negative");
    }
    const std::size_t count = static_cast<std::size_t>(number);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> norm_uniform(0, 1);

    OscillatorSoABatch_no_termination soa_batch = {
        .position = std::vector<double>(count),
        .velocity = std::vector<double>(count),
        .m00 = std::vector<double>(count),
        .m01 = std::vector<double>(count),
        .m10 = std::vector<double>(count),
        .m11 = std::vector<double>(count),
        .omega = std::vector<double>(count),
        .zeta = std::vector<double>(count),
    };

    UnderdampedOscillator system(omega, zeta);
    StepCoefficients step_coefficients = system.make_step_coefficients(dt);

    for (int i = 0; i < number; ++i) {
        soa_batch.omega[i] = omega;
        soa_batch.zeta[i] = zeta;

        double theta = norm_uniform(gen) * 2 * std::numbers::pi;
        double r = sqrt(norm_uniform(gen));
        soa_batch.position[i] = r * cos(theta);
        soa_batch.velocity[i] = r * sin(theta);

        soa_batch.m00[i] = step_coefficients.m00;
        soa_batch.m01[i] = step_coefficients.m01;
        soa_batch.m10[i] = step_coefficients.m10;
        soa_batch.m11[i] = step_coefficients.m11;
    }

    return soa_batch;
}

// ---- 无终止SOA振子更新 --------------------------------------------------------------

void update_soa_batch_step_no_termination(OscillatorSoABatch_no_termination& soa_batch) {
    const std::size_t count = soa_batch.omega.size();
    for (std::size_t i = 0; i < count; ++i) {
        std::size_t batch_index = i;
        const double old_position = soa_batch.position[batch_index];
        const double old_velocity = soa_batch.velocity[batch_index];
        soa_batch.position[batch_index] =
            soa_batch.m00[batch_index] * old_position + soa_batch.m01[batch_index] * old_velocity;
        soa_batch.velocity[batch_index] =
            soa_batch.m10[batch_index] * old_position + soa_batch.m11[batch_index] * old_velocity;
    }
}

void update_soa_batch_step_no_termination(OscillatorSoABatch_no_termination& soa_batch, int begin,
                                          int end) {
    for (std::size_t i = begin; i < end; ++i) {
        std::size_t batch_index = i;
        const double old_position = soa_batch.position[batch_index];
        const double old_velocity = soa_batch.velocity[batch_index];
        soa_batch.position[batch_index] =
            soa_batch.m00[batch_index] * old_position + soa_batch.m01[batch_index] * old_velocity;
        soa_batch.velocity[batch_index] =
            soa_batch.m10[batch_index] * old_position + soa_batch.m11[batch_index] * old_velocity;
    }
}

void update_soa_batch_no_termination(OscillatorSoABatch_no_termination& soa_batch, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    for (int i = 0; i < step; ++i) {
        update_soa_batch_step_no_termination(soa_batch);
    };
}

void update_soa_batch_no_termination_range(OscillatorSoABatch_no_termination& soa_batch, int begin,
                                           int end, int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    if (begin < 0 || end < 0) {
        throw std::invalid_argument("index must be non-negative");
    }

    if (begin > end) {
        throw std::invalid_argument("begin must < end");
    }

    int number_oscillator = soa_batch.omega.size();

    if (end > number_oscillator) {
        throw std::invalid_argument("end must <= number of oscillator");
    }

    for (int i = 0; i < step; ++i) {
        update_soa_batch_step_no_termination(soa_batch, begin, end);
    };
}

void update_soa_batch_no_termination_parallel(OscillatorSoABatch_no_termination& soa_batch,
                                              const int steps, const std::size_t thread_count) {
    if (steps < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    if (thread_count <= 0) {
        throw std::invalid_argument("thread_cout must be positive");
    }

    const std::size_t number_batch = soa_batch.omega.size();

    const std::size_t thread_require = std::min(number_batch, thread_count);
    if (thread_require == 0) {
        return;
    }

    std::size_t number_range = number_batch / thread_require;
    const std::size_t remainder = number_batch % thread_require;
    std::size_t number_range_remainder = number_range + 1;

    std::vector<std::jthread> workers;

    std::size_t begin_index = number_range;
    std::size_t end_index;

    for (std::size_t i = 1; i < thread_require; ++i) {
        if (i <= remainder) {
            end_index = begin_index + number_range_remainder;
        } else {
            end_index = begin_index + number_range;
        }

        const std::size_t worker_index = i - 1;
        workers.emplace_back([&soa_batch, begin_index, end_index, steps] {
            update_soa_batch_no_termination_range(soa_batch, begin_index, end_index, steps);
        });

        begin_index = end_index;
    }

    update_soa_batch_no_termination_range(soa_batch, 0, number_range, steps);

    for (auto& worker : workers) {
        worker.join();
    }
}

void update_soa_batch_step_no_termination_scalar(OscillatorSoABatch_no_termination& soa_batch) {
    const std::size_t count = soa_batch.omega.size();

#pragma loop(no_vector)
    for (std::size_t i = 0; i < count; ++i) {
        std::size_t batch_index = i;
        const double old_position = soa_batch.position[batch_index];
        const double old_velocity = soa_batch.velocity[batch_index];
        soa_batch.position[batch_index] =
            soa_batch.m00[batch_index] * old_position + soa_batch.m01[batch_index] * old_velocity;
        soa_batch.velocity[batch_index] =
            soa_batch.m10[batch_index] * old_position + soa_batch.m11[batch_index] * old_velocity;
    }
}

void update_soa_batch_no_termination_scalar(OscillatorSoABatch_no_termination& soa_batch,
                                            int step) {
    if (step < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    for (int i = 0; i < step; ++i) {
        update_soa_batch_step_no_termination_scalar(soa_batch);
    };
}

// ---- SOA振子输出相关信息 --------------------------------------------------------------

BatchResults soa_batch_report(const OscillatorSoABatch& soa_batch_updated) {
    std::size_t N = soa_batch_updated.omega.size();
    double state_checksum = 0.0;
    double max_abs_x = 0.0;
    double max_abs_v = 0.0;
    bool finite = 1;
    for (std::size_t i = 0; i < N; ++i) {
        // checksum 压缩全部状态，max 值和 finite 则帮助识别发散或非有限结果。
        state_checksum += soa_batch_updated.position[i] + 0.5 * soa_batch_updated.velocity[i];
        if (std::abs(soa_batch_updated.position[i]) > max_abs_x) {
            max_abs_x = std::abs(soa_batch_updated.position[i]);
        }
        if (std::abs(soa_batch_updated.velocity[i]) > max_abs_v) {
            max_abs_v = std::abs(soa_batch_updated.velocity[i]);
        }

        if (!std::isfinite(soa_batch_updated.position[i]) ||
            !std::isfinite(soa_batch_updated.velocity[i])) {
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

BatchResults soa_batch_report(const OscillatorSoABatch_no_termination& soa_batch_updated) {
    std::size_t N = soa_batch_updated.omega.size();
    double state_checksum = 0.0;
    double max_abs_x = 0.0;
    double max_abs_v = 0.0;
    bool finite = 1;
    for (std::size_t i = 0; i < N; ++i) {
        // checksum 压缩全部状态，max 值和 finite 则帮助识别发散或非有限结果。
        state_checksum += soa_batch_updated.position[i] + 0.5 * soa_batch_updated.velocity[i];
        if (std::abs(soa_batch_updated.position[i]) > max_abs_x) {
            max_abs_x = std::abs(soa_batch_updated.position[i]);
        }
        if (std::abs(soa_batch_updated.velocity[i]) > max_abs_v) {
            max_abs_v = std::abs(soa_batch_updated.velocity[i]);
        }

        if (!std::isfinite(soa_batch_updated.position[i]) ||
            !std::isfinite(soa_batch_updated.velocity[i])) {
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
