#include "state.h"
#include "explicit_euler_step.h"
#include <numbers>

namespace oscillator {
    State explicit_euler_step(double time, const State& current_state, const HarmonicOscillator& system) {
    double cycle = 2 * std::numbers::pi / system.angular_frequency();
        double dt;
    }
}
