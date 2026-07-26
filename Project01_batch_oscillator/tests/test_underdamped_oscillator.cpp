#include "test_underdamped_oscillator.h"
#include "underdamped_oscillator.h"
#include "state.h"
#include "harmonic_oscillator.h"
#include "test_common.h"


#include <cmath>
#include <numbers>


namespace oscillator {

void test_underdamped_exact_state() {
    // 检查解析公式在 t=0 时能否严格恢复给定的初始状态。
    const double omega = 2.0 * std::numbers::pi;
    const double damp = 0.5;
    const double beta = omega * damp;
    const double cycle = 2.0 * std::numbers::pi / omega;
    const double exp_beta = std::exp(-beta * cycle);

    const UnderdampedOscillator system(omega, damp);
    const State initial_state{.position = 1.25, .velocity = -0.7};
    const State T_state{.position = initial_state.position * exp_beta,
                        .velocity = initial_state.velocity * exp_beta};
    expect_state_near(system.exact_state(0.0, initial_state), initial_state, "exact state at t=0");
    //expect_state_near(system.exact_state(cycle, initial_state), T_state, "exact state at t=T");
}

void test_underdamped_exact_state_step() {
    // 使用较短时间分别检查一次更新和连续多次更新，避免强衰减掩盖误差。
    const double omega = 2.0 * std::numbers::pi;
    const double damp = 0.5;
    const int step1 = 1;
    const int step2 = 10;
    const double dt = 0.01;
    const double time1 = step1 * dt;
    const double time2 = step2 * dt;

    const UnderdampedOscillator system(omega, damp);
    const State initial_state{.position = 1.25, .velocity = -0.7};
    // 系统参数和 dt 在两个循环中相同，因此共用同一组预计算系数。
    const StepCoefficients step_coefficients = system.make_step_coefficients(dt);
    State current_state1 = initial_state;
    State current_state2 = initial_state;

    for (int i = 0; i < step1; ++i)
    {
        current_state1 = system.exact_state_step(step_coefficients, current_state1);
    }

    // 一步矩阵更新应与同一时刻的直接解析解一致。
    expect_state_near(system.exact_state(time1, initial_state), current_state1,
                      "exact state full calu vs step calu");

    for (int i = 0; i < step2; ++i) {
        current_state2 = system.exact_state_step(step_coefficients, current_state2);
    }

    // 重复应用同一矩阵后，结果仍应与直接解析解一致。
    expect_state_near(system.exact_state(time2, initial_state), current_state2,
                      "exact state full calu vs step calu");
}

void test_zero_damping()
{
    // zeta=0 时指数衰减消失，欠阻尼解析解应退化为无阻尼简谐振子。
    const double omega = 2.0 * std::numbers::pi;
    const double damp = 0.0;
    const double cycle = 2.0 * std::numbers::pi / omega;

    const State initial_state{.position = 1.25, .velocity = -0.7};
    const UnderdampedOscillator system1(omega, damp);
    const HarmonicOscillator system2(omega);

    expect_state_near(system1.exact_state(0.0, initial_state),
                      system2.exact_state(0.0, initial_state), "damp vs harmonic at t=0");
    expect_state_near(system1.exact_state(cycle, initial_state),
                      system2.exact_state(cycle, initial_state), "damp vs harmonic at t=T");
    
}

}
