#pragma once

namespace oscillator {
// 验证解析解在初始时刻和一个完整周期后的状态。
void test_exact_state();

// 验证状态导数和单位质量机械能的已知结果。
void test_derivative_energy();

// 验证各时间积分器的单步结果和 Position Verlet 状态转换。
void test_time_integrators();
}
