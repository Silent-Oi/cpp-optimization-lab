#include "state.h"
#include "underdamped_oscillator.h"
#include <numbers>
#include <iostream>

int main() {
    // 实验参数
    const double omega = 1.0 * std::numbers::pi;
    const double zeta = 0.8;
    const int step = 130000;
    const double dt = 0.013;
    const double time = step * dt;

    // 初始化测试振子，并为固定 dt 预计算一次精确更新矩阵。
    const oscillator::UnderdampedOscillator system(omega, zeta);
    const oscillator::State initial_state{.position = 1.4, .velocity = -0.82};
    oscillator::State current_state = initial_state;
    const oscillator::StepCoefficients step_coefficients = system.make_step_coefficients(dt);

    // 分别使用任意时刻解析解和重复精确一步更新计算同一终止时刻。
    oscillator::State final_state = system.exact_state(time, initial_state);
    for (int i = 0; i < step; ++i) {
        current_state = system.exact_state_step(step_coefficients, current_state);
    }

    // 记录初始、直接解析和分步更新三种状态对应的机械能，便于人工核对。
    const double initial_energy = system.system_energy(initial_state);
    const double final_energy = system.system_energy(final_state);
    const double current_energy = system.system_energy(current_state);

    std::cout << "angular_frequency: " << omega << '\n';
    std::cout << "damping: " << zeta << '\n';
    std::cout << "initial_position: " << initial_state.position << '\n';
    std::cout << "initial_velocity: " << initial_state.velocity << '\n';
    std::cout << "initial_energy: " << initial_energy << '\n';
    std::cout << "dt: " << dt << '\n';
    std::cout << "step: " << step << '\n';
    std::cout << "time: " << time << '\n';
    std::cout << '\n';
    // 第一组输出来自任意时刻解析解。
    std::cout << "position: " << final_state.position << '\n';
    std::cout << "velocity: " << final_state.velocity << '\n';
    std::cout << "energy: " << final_energy << '\n';
    std::cout << '\n';
    // 第二组输出来自重复应用预计算状态转移矩阵。
    std::cout << "step_position: " << current_state.position << '\n';
    std::cout << "step_velocity: " << current_state.velocity << '\n';
    std::cout << "step_energy: " << current_energy << '\n';
    std::cout << '\n';
}
