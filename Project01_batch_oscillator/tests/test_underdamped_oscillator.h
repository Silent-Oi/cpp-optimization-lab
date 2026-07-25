#pragma once

namespace oscillator {

// 验证欠阻尼解析解在初始时刻恢复给定状态。
void test_underdamped_exact_state();

// 验证预计算状态转移矩阵的一步和多步结果。
void test_underdamped_exact_state_step();

// 验证 zeta=0 时欠阻尼模型退化为无阻尼谐振子。
void test_zero_damping();

}  // namespace oscillator
