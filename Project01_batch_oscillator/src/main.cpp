#include "state.h"
#include "harmonic_oscillator.h"
#include <numbers>
#include <iostream>
#include "time_integrators.h"
#include "test_harmonic_oscillator.h"

int main()
{
    oscillator::test_derivative_energy();
    oscillator::test_exact_state();
    oscillator::test_time_integrators();

    // 实验参数：dt 取一个振动周期的固定比例。
    const double omega = 2.0 * std::numbers::pi;
    const double cycle = 2 * std::numbers::pi / omega;
    const int steps = 123456;
    const double fit = 0.001;
    const double dt = fit * cycle;
    const double time = dt * steps;

    const oscillator::HarmonicOscillator system(omega);
    const oscillator::State initial_state{.position = 1.0, .velocity = 0.0};

    // 所有积分器从相同物理状态开始，保证比较公平。
    oscillator::State explicit_euler_state = initial_state;
    oscillator::State symplectic_euler_state = initial_state;
    oscillator::State velocity_verlet_state = initial_state;
    oscillator::State position_verlet_initial_state = initial_state;

    // Position Verlet 保存 {x_1, x_0}，初始化过程已经完成第一个时间步。
    oscillator::PositionVerletState position_verlet_state =
        oscillator::initialize_position_verlet(dt, position_verlet_initial_state, system);

    // 使用相同终止时刻的解析解作为参考结果。
    const oscillator::State state = system.exact_state(time, initial_state);
    for (int i = 0; i < steps; ++i)
    {
        explicit_euler_state = oscillator::explicit_euler_step(dt, explicit_euler_state, system);
        symplectic_euler_state =
            oscillator::symplectic_velocity_euler_step(dt, symplectic_euler_state, system);
        velocity_verlet_state = oscillator::velocity_verlet_step(dt, velocity_verlet_state, system);

        // i == 0 时 Position Verlet 已位于 x_1，此时只推进其他方法。
        if (i == 0)
        {
            continue;
        }
        position_verlet_state = oscillator::position_verlet_step(dt, position_verlet_state, system);
    }

    // 将 {x_steps, x_{steps-1}} 恢复为位置-速度状态，以便统一比较。
    oscillator::State position_verlet_final_state =
        oscillator::state_position_verlet(dt, position_verlet_state, system);

    // 同时比较状态误差和能量误差；仅能量相同无法发现相位误差。
    double energy = system.system_energy(state);
    double explicit_euler_energy = system.system_energy(explicit_euler_state);
    double symplectic_euler_energy = system.system_energy(symplectic_euler_state);
    double velocity_verlet_energy = system.system_energy(velocity_verlet_state);
    double position_verlet_energy = system.system_energy(position_verlet_final_state);

    const oscillator::State error_explicit_euler{
        .position = state.position - explicit_euler_state.position,
        .velocity = state.velocity - explicit_euler_state.velocity};
    const oscillator::State error_symplectic_euler{
        .position = state.position - symplectic_euler_state.position,
        .velocity = state.velocity - symplectic_euler_state.velocity};
    const oscillator::State error_velocity_verlet{
        .position = state.position - velocity_verlet_state.position,
        .velocity = state.velocity - velocity_verlet_state.velocity};
    const oscillator::State error_position_verlet{
        .position = state.position - position_verlet_final_state.position,
        .velocity = state.velocity - position_verlet_final_state.velocity};
    double error_explicit_euler_energy = energy - explicit_euler_energy;
    double error_symplectic_euler_energy = energy - symplectic_euler_energy;
    double error_velocity_verlet_energy = energy - velocity_verlet_energy;
    double error_position_verlet_energy = energy - position_verlet_energy;

    // 先输出解析参考值，再输出各积分器的带符号误差。
    std::cout << "time: " << time << '\n';
    std::cout << "position: " << state.position << '\n';
    std::cout << "velocity: " << state.velocity << '\n';
    std::cout << "energy: " << energy << '\n';
    std::cout << '\n';
    std::cout << "explicit_euler_position: " << explicit_euler_state.position << '\n';
    std::cout << "explicit_euler_velocity: " << explicit_euler_state.velocity << '\n';
    std::cout << "explicit_euler_energy: " << explicit_euler_energy << '\n';
    std::cout << "error_explicit_euler_position: " << error_explicit_euler.position << '\n';
    std::cout << "error_explicit_euler_velocity: " << error_explicit_euler.velocity << '\n';
    std::cout << "error_explicit_euler_energy: " << error_explicit_euler_energy << '\n';
    std::cout << '\n';
    std::cout << "symplectic_euler_position: " << symplectic_euler_state.position << '\n';
    std::cout << "symplectic_euler_velocity: " << symplectic_euler_state.velocity << '\n';
    std::cout << "symplectic_euler_energy: " << symplectic_euler_energy << '\n';
    std::cout << "error_symplectic_euler_position: " << error_symplectic_euler.position << '\n';
    std::cout << "error_symplectic_euler_velocity: " << error_symplectic_euler.velocity << '\n';
    std::cout << "error_symplectic_euler_energy: " << error_symplectic_euler_energy << '\n';
    std::cout << '\n';
    std::cout << "velocity_verlet_position: " << velocity_verlet_state.position << '\n';
    std::cout << "velocity_verlet_velocity: " << velocity_verlet_state.velocity << '\n';
    std::cout << "velocity_verlet_energy: " << velocity_verlet_energy << '\n';
    std::cout << "error_velocity_verlet_position: " << error_velocity_verlet.position << '\n';
    std::cout << "error_velocity_verlet_velocity: " << error_velocity_verlet.velocity << '\n';
    std::cout << "error_velocity_verlet_energy: " << error_velocity_verlet_energy << '\n';
    std::cout << '\n';
    std::cout << "position_verlet_position: " << position_verlet_final_state.position << '\n';
    std::cout << "position_verlet_velocity: " << position_verlet_final_state.velocity << '\n';
    std::cout << "position_verlet_energy: " << position_verlet_energy << '\n';
    std::cout << "error_position_verlet_position: " << error_position_verlet.position << '\n';
    std::cout << "error_position_verlet_velocity: " << error_position_verlet.velocity << '\n';
    std::cout << "error_position_verlet_energy: " << error_position_verlet_energy << '\n';
    std::cout << '\n';

}
