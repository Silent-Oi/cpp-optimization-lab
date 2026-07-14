#pragma once
#include "state.h"
#include "harmonic_oscillator.h"

namespace oscillator {
State explicit_euler_step(double time, const State& current_state, const HarmonicOscillator& system);
}
