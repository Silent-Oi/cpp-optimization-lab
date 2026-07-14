#include "harmonic_oscillator.h"
#include <cmath>
#include <stdexcept>
#include "state.h"


namespace oscillator {
    HarmonicOscillator::HarmonicOscillator(double angular_frequency)
    : angular_frequency_{angular_frequency}
    {
    if (!std::isfinite(angular_frequency) || angular_frequency <= 0.0) {
        throw std::invalid_argument("angular_frequency must be positive");
    }
    }

    double HarmonicOscillator::angular_frequency() const {
        return angular_frequency_;
    }


    State HarmonicOscillator::exact_state(double time, const State& initial_state) const
    {
        State state;
        double velocity_over_omega = initial_state.velocity / angular_frequency_;
        double angle = angular_frequency_ * time;
        double cos_angle = std::cos(angle);
        double sin_angle = std::sin(angle);
        state.position = initial_state.position * cos_angle + velocity_over_omega * sin_angle;
        state.velocity = -1 * initial_state.position * angular_frequency_ * sin_angle +
                         initial_state.velocity * cos_angle;
        return state;
    }

    State HarmonicOscillator::derivative(const State& state) const
    {
        return {
            .position = state.velocity,
                .velocity = -angular_frequency_ * angular_frequency_ * state.position};
    }


}

