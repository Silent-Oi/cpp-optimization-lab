#include "state.h"
#include "time_integrators.h"
#include "harmonic_oscillator.h"

namespace oscillator {
State explicit_euler_step(double dt, const State& current_state, const HarmonicOscillator& system) {
    // 显式 Euler 的两个变化率都在旧状态 y_n 上计算。
    const StateDerivative rate = system.derivative(current_state);

    double position_step = dt * rate.position_rate + current_state.position;
    double velocity_step = dt * rate.velocity_rate + current_state.velocity;

    return State{position_step, velocity_step};
}

State symplectic_velocity_euler_step(double dt, const State& current_state,
                                     const HarmonicOscillator& system) {
    // 速度优先辛 Euler：先用 x_n 计算 v_{n+1}，再用 v_{n+1} 计算 x_{n+1}。
    const StateDerivative rate = system.derivative(current_state);
    double velocity_step = dt * rate.velocity_rate + current_state.velocity;
    double position_step = dt * velocity_step + current_state.position;
    return State{position_step, velocity_step};
}

State velocity_verlet_step(double dt, const State& current_state, const HarmonicOscillator& system) {
    // 先使用 a_n 更新位置，再使用 a_n 与 a_{n+1} 的平均值更新速度。
    // 对 a(x) = -omega^2*x，两次加速度之和为 -omega^2*(x_n + x_{n+1})。
    const StateDerivative rate = system.derivative(current_state);
    double position_step = current_state.position + dt * current_state.velocity + 0.5 * dt * dt * rate.velocity_rate;
    double velocity_step = -0.5 * system.angular_frequency() * system.angular_frequency() * dt * (current_state.position + position_step) + current_state.velocity;
    return State{position_step, velocity_step};
}

PositionVerletState position_verlet_step(double dt, const PositionVerletState& current_state, const HarmonicOscillator& system) {
    // x_{n+1} = 2*x_n - x_{n-1} + dt^2*a_n。
    double position_step = 2 * current_state.current_position - current_state.previous_position -
                           dt * dt * system.angular_frequency() * system.angular_frequency() *
                               current_state.current_position;
    // 将保存的时间层从 {x_n, x_{n-1}} 移动为 {x_{n+1}, x_n}。
    return PositionVerletState {position_step, current_state.current_position};
}

PositionVerletState initialize_position_verlet(double dt, const State& initial_state, const HarmonicOscillator& system) {
    // Position Verlet 需要两个位置；使用二阶 Taylor 展开得到 x_1。
    const StateDerivative rate = system.derivative(initial_state);
    double position_step = initial_state.position + dt * initial_state.velocity + 0.5 * dt * dt * rate.velocity_rate;
    return PositionVerletState{position_step, initial_state.position};
}

State state_position_verlet(double dt, const PositionVerletState& current_state, const HarmonicOscillator& system) {
    // 根据 x_n 和 x_{n-1} 恢复 v_n：
    // v_n = (x_n - x_{n-1})/dt + 0.5*dt*a_n.
    double velocity_state =
        (current_state.current_position - current_state.previous_position) / dt -
        0.5 * dt * system.angular_frequency() * system.angular_frequency() *
            current_state.current_position;
    return State {current_state.current_position, velocity_state};
}

PositionVerletState advance_position_verlet(double dt, int steps, const State& initial_state,
    const HarmonicOscillator& system) {
    // 初始化已经从 x_0 推进到 x_1，因此只需继续递推 steps - 1 次。
    PositionVerletState state = initialize_position_verlet(dt, initial_state, system);
    // 简谐振子的递推系数在所有时间步中保持不变。
    const double param = 2 - dt * dt * system.angular_frequency() * system.angular_frequency();
    for (int i = 0; i < steps - 1; ++i) {
        double position_step = param * state.current_position - state.previous_position;
        state = {position_step, state.current_position};
    }
    return state;   
}

}
