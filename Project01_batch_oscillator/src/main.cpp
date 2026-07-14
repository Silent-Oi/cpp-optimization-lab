#include "state.h"
#include "harmonic_oscillator.h"
#include <numbers>
#include <iostream>

int main()
{
    const double omega = 2.0 * std::numbers::pi;
    const oscillator::HarmonicOscillator system(omega);
    const oscillator::State initial_state{.position = 1.0, .velocity = 0.0};
    const double time = 1.25;

    const oscillator::State state = system.exact_state(time, initial_state);

    std::cout << "time: " << time << '\n';
    std::cout << "position: " << state.position << '\n';
    std::cout << "velocity: " << state.velocity << '\n';
}
