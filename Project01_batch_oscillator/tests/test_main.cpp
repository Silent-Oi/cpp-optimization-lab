#include "test_harmonic_oscillator.h"
#include "test_underdamped_oscillator.h"


int main() {
    // 无阻尼模型及时间积分器回归测试。
    oscillator::test_harmonic_derivative_energy();
    oscillator::test_harmonic_exact_state();
    oscillator::test_harmonic_time_integrators();

    // 欠阻尼解析解、精确一步更新及零阻尼退化测试。
    oscillator::test_underdamped_exact_state();
    oscillator::test_underdamped_exact_state_step();
    oscillator::test_zero_damping();

}
