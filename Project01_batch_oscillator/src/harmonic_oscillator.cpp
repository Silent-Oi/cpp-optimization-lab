#include "harmonic_oscillator.h"
#include <cmath>
#include <stdexcept>
#include "state.h"

namespace oscillator {
    HarmonicOscillator::HarmonicOscillator(double angular_frequency)
    : angular_frequency_{angular_frequency}
    {
    // 解析解和所有积分器都要求 omega 有限且不为零。
    if (!std::isfinite(angular_frequency) || angular_frequency <= 0.0) {
        throw std::invalid_argument("angular_frequency must be positive");
    }
    }

    double HarmonicOscillator::angular_frequency() const {
        return angular_frequency_;
    }

    // x'' = -omega^2 * x 对应的单位质量机械能。
    double HarmonicOscillator::system_energy(const State& state) const {
        return 0.5 * state.velocity * state.velocity +
               0.5 * angular_frequency_ * angular_frequency_ * state.position * state.position;
    }


    State HarmonicOscillator::exact_state(double time, const State& initial_state) const
    {
        // x(t) = x0*cos(omega*t) + (v0/omega)*sin(omega*t)。
        // 复用 sin 和 cos，保证位置与速度使用同一次相位计算结果。
        double velocity_over_omega = initial_state.velocity / angular_frequency_;
        double angle = angular_frequency_ * time;
        double cos_angle = std::cos(angle);
        double sin_angle = std::sin(angle);
        double position = initial_state.position * cos_angle + velocity_over_omega * sin_angle;
        double velocity = -1 * initial_state.position * angular_frequency_ * sin_angle +
                         initial_state.velocity * cos_angle;
        return State{position, velocity};
    }

    StateDerivative HarmonicOscillator::derivative(const State& state) const {
        // 一阶形式：dx/dt = v，dv/dt = -omega^2*x。
        return{
            .position_rate = state.velocity,
            .velocity_rate = -angular_frequency_ * angular_frequency_ * state.position};
    }


}
