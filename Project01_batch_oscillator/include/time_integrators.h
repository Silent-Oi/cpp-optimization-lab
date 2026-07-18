#pragma once
#include "state.h"
#include "harmonic_oscillator.h"

namespace oscillator {
// 显式 Euler 单步推进；新位置和新速度都使用旧状态计算。
State explicit_euler_step(double dt, const State& current_state, const HarmonicOscillator& system);

// 速度优先的辛 Euler 单步推进。
State symplectic_velocity_euler_step(double dt, const State& current_state,
                                    const HarmonicOscillator& system);

// Velocity Verlet 单步推进，返回 x_{n+1} 和 v_{n+1}。
State velocity_verlet_step(double dt, const State& current_state, const HarmonicOscillator& system);

// Position Verlet 单步推进：{x_n, x_{n-1}} -> {x_{n+1}, x_n}。
PositionVerletState position_verlet_step(double dt, const PositionVerletState& current_state, const HarmonicOscillator& system);

// 根据 {x_0, v_0} 构造 Position Verlet 初始位置对 {x_1, x_0}。
PositionVerletState initialize_position_verlet(double dt, const State& initial_state, const HarmonicOscillator& system);

// 根据 {x_n, x_{n-1}} 恢复 v_n，并返回 t_n 时刻的状态。
State state_position_verlet(double dt, const PositionVerletState& current_state, const HarmonicOscillator& system);

// 从初始状态推进到 {x_steps, x_{steps-1}}。
// 前置条件：steps >= 1，并且 dt 有限且不为零。
PositionVerletState advance_position_verlet(double dt, int steps, const State& initial_state,
                               const HarmonicOscillator& system);

}
