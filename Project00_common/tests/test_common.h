#pragma once
#include <string>


namespace common {

// 使用统一绝对容差比较标量；失败时抛出包含期望值和实际值的异常。
void expect_near(double actual, double expected, double tolerance);

}  // namespace oscillator
