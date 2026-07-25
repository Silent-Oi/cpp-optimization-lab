#include "state.h"
#include "underdamped_oscillator.h"
#include <stdexcept>
#include <cmath>


namespace oscillator
{
    UnderdampedOscillator::UnderdampedOscillator(double angular_frequency, double damping)
    : angular_frequency_{angular_frequency},
      damping_{damping},
      // omega_d 控制振荡相位，beta 控制包络的指数衰减。
      omega_d_{angular_frequency * std::sqrt(1 - damping * damping)},
      beta_{angular_frequency * damping}
    {
    if (!std::isfinite(angular_frequency) || angular_frequency <= 0.0) {
        throw std::invalid_argument("angular_frequency must be positive");
    }
    if (damping >= 1.0 || damping < 0.0 || std::isnan(damping)) {
        throw std::invalid_argument("damping must in [0,1)");
    }
    }

    double UnderdampedOscillator::angular_frequency() const
    {
        return angular_frequency_;
    }
    double UnderdampedOscillator::damping() const
    {
        return damping_;
    }

    double UnderdampedOscillator::system_energy(const State& state) const
    {
        return 0.5 * state.velocity * state.velocity +
               0.5 * angular_frequency_ * angular_frequency_ * state.position * state.position;
    }
    double UnderdampedOscillator::system_energy(double time, const State& initial_state) const
    {
        const State final_state = exact_state(time, initial_state);
        return system_energy(final_state);
    }

    StepCoefficients UnderdampedOscillator::make_step_coefficients(double dt) const
    {
        // 三角函数和指数函数仅依赖系统参数及 dt，因此只需在系数生成时计算一次。
        const double angle = omega_d_ * dt;
        const double sin_angle = std::sin(angle);
        const double cos_angle = std::cos(angle);
        const double exp_beta = std::exp(-beta_ * dt);

        // M 的两行分别给出下一步位置和速度对当前位置、速度的线性组合。
        return StepCoefficients{
            .m00 = exp_beta * (cos_angle + sin_angle * beta_ / omega_d_),
            .m01 = exp_beta * (sin_angle / omega_d_),
            .m10 = exp_beta * -(angular_frequency_ * angular_frequency_ * sin_angle / omega_d_),
            .m11 = exp_beta * (cos_angle - beta_ * sin_angle / omega_d_)};
    }


    State UnderdampedOscillator::exact_state(double time, const State& initial_state) const
    {
        // x(t) = exp(-beta*t) * (A*cos(omega_d*t) + B*sin(omega_d*t))。
        // A、B 由初始位置和初始速度唯一确定。
        const double para_A = initial_state.position;
        const double para_B = (initial_state.velocity + beta_ * para_A) / omega_d_;

        const double angle = omega_d_ * time;
        const double sin_angle = std::sin(angle);
        const double cos_angle = std::cos(angle);
        const double exp_beta = std::exp(-beta_ * time);

        const double position = exp_beta * (para_A * cos_angle + para_B * sin_angle);
        const double velocity = exp_beta * ((-beta_ * para_A + omega_d_ * para_B) * cos_angle -
                                            (beta_ * para_B + omega_d_ * para_A) * sin_angle);
        return State{position, velocity};
    }

    State UnderdampedOscillator::exact_state_step(const StepCoefficients& step_coefficients,
                                                  const State& current_state) const {
        // 展开二维矩阵与状态向量的乘法，避免每一步重复计算超越函数。
        const double position =
            step_coefficients.m00 * current_state.position +
                        step_coefficients.m01 *
                            current_state.velocity;
        const double velocity =
                 step_coefficients.m10 *
                     current_state.position +
                        step_coefficients.m11 * current_state.velocity;
        return State{position, velocity};
    }

    }
