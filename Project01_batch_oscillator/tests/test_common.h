#pragma once
#include <string>
#include "state.h"

namespace oscillator {

// 使用统一绝对容差比较标量；失败时抛出包含期望值和实际值的异常。
void expect_near(double actual, double expected, const std::string& quantity);

// 分别比较 State 的位置和速度，并在错误信息中保留测试名称。
void expect_state_near(const State& actual, const State& expected, const std::string& test_name);

// 逐字段比较 OscillatorAoS 的状态、原始参数和派生系数。
void expect_aos_near(const OscillatorAoS& actual, const OscillatorAoS& expected,
                           const std::string& test_name);

}  // namespace oscillator
