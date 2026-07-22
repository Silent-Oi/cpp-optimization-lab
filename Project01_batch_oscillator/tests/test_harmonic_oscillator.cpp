#include "harmonic_oscillator.h"
#include "state.h"
#include "test_harmonic_oscillator.h"
#include "time_integrators.h"

#include <cmath>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>

namespace oscillator {
namespace {

constexpr double tolerance = 1e-12;

// 使用统一绝对容差比较标量；失败时抛出包含期望值和实际值的异常。
void expect_near(double actual, double expected, const std::string& quantity) {
    if (std::abs(actual - expected) <= tolerance) {
        return;
    }

    std::ostringstream message;
    message.precision(17);
    message << quantity << ": expected " << expected << ", got " << actual;
    throw std::runtime_error(message.str());
}

// 分别比较 State 的位置和速度，并在错误信息中保留测试名称。
void expect_state_near(const State& actual, const State& expected,
                       const std::string& test_name) {
    expect_near(actual.position, expected.position, test_name + " position");
    expect_near(actual.velocity, expected.velocity, test_name + " velocity");
}

}  // namespace

void test_exact_state() {
    const double omega = 2.0 * std::numbers::pi;
    const double cycle = 2.0 * std::numbers::pi / omega;
    const HarmonicOscillator system(omega);
    const State initial_state{.position = 1.25, .velocity = -0.7};

    expect_state_near(system.exact_state(0.0, initial_state), initial_state,
                      "exact state at t=0");
    expect_state_near(system.exact_state(cycle, initial_state), initial_state,
                      "exact state after one period");
}

void test_derivative_energy() {
    const HarmonicOscillator system(2.0);
    const State state{.position = 3.0, .velocity = -4.0};

    const StateDerivative derivative = system.derivative(state);
    expect_near(derivative.position_rate, -4.0, "position derivative");
    expect_near(derivative.velocity_rate, -12.0, "velocity derivative");
    expect_near(system.system_energy(state), 26.0, "mechanical energy");
}

void test_time_integrators() {
    const double dt = 0.1;
    const HarmonicOscillator system(2.0);
    const State initial_state{.position = 3.0, .velocity = -4.0};

    expect_state_near(explicit_euler_step(dt, initial_state, system),
                      State{.position = 2.6, .velocity = -5.2},
                      "explicit Euler step");
    expect_state_near(symplectic_velocity_euler_step(dt, initial_state, system),
                      State{.position = 2.48, .velocity = -5.2},
                      "symplectic Euler step");
    expect_state_near(velocity_verlet_step(dt, initial_state, system),
                      State{.position = 2.54, .velocity = -5.108},
                      "velocity Verlet step");

    const PositionVerletState initialized =
        initialize_position_verlet(dt, initial_state, system);
    expect_near(initialized.current_position, 2.54,
                "position Verlet initialized current position");
    expect_near(initialized.previous_position, 3.0,
                "position Verlet initialized previous position");

    const PositionVerletState stepped =
        position_verlet_step(dt, PositionVerletState{3.0, 2.5}, system);
    expect_near(stepped.current_position, 3.38,
                "position Verlet step current position");
    expect_near(stepped.previous_position, 3.0,
                "position Verlet step previous position");

    expect_state_near(state_position_verlet(dt, initialized, system),
                      State{.position = 2.54, .velocity = -5.108},
                      "position Verlet reconstructed state");

    const PositionVerletState advanced =
        advance_position_verlet(dt, 1, initial_state, system);
    expect_near(advanced.current_position, 2.54,
                "position Verlet one-step advance current position");
    expect_near(advanced.previous_position, 3.0,
                "position Verlet one-step advance previous position");
}

}  // namespace oscillator
