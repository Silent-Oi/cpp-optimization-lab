#include <stdexcept>

#include "oscillator_batch.h"
#include "state.h"
#include "test_common.h"
#include "underdamped_oscillator.h"

namespace oscillator {

// 从批量元素保存的初始状态和原始参数构造 M1 标量解析参考。
// 参考状态直接计算到绝对时间 time，不复用待测的批量更新路径。
OscillatorAoS system_to_aos(const OscillatorAoS& oscillator, double time, double dt) {
    UnderdampedOscillator system(oscillator.omega, oscillator.zeta);
    State initail_state = {.position = oscillator.position, .velocity = oscillator.velocity};
    StepCoefficients step_cofficients = system.make_step_coefficients(dt);
    State final_state = system.exact_state(time, initail_state);

    return OscillatorAoS{.position = final_state.position,
                         .velocity = final_state.velocity,
                         .omega = oscillator.omega,
                         .zeta = oscillator.zeta,
                         .m00 = step_cofficients.m00,
                         .m01 = step_cofficients.m01,
                         .m10 = step_cofficients.m10,
                         .m11 = step_cofficients.m11};
}

void test_batch_number() {
    // 初始化接口应支持调用者指定的任意非负批量规模。
    const double dt = 0.1;
    const int number = 10;
    const int seed = 12345;
    OscillatorBatch oscillator_aos_batch = make_OscillatorAoS_batch(number, dt, seed);

    const std::size_t i = number;

    if (oscillator_aos_batch.size() == i) {
        return;
    }

    throw std::runtime_error("振子数量错误");
}

void test_zero_batch_and_update() {
    // N=0 时，无论推进零步还是多步都应保持为空且不报错。
    const double dt = 0.1;
    const int number = 0;
    const int step1 = 0;
    const int step2 = 10;
    const int seed = 12345;
    OscillatorBatch oscillator_aos_batch = make_OscillatorAoS_batch(number, dt, seed);
    update_aos_batch(oscillator_aos_batch, step1);
    update_aos_batch(oscillator_aos_batch, step2);

    if (oscillator_aos_batch.size() == 0)
    {
        return;
    }
    throw std::runtime_error("空振子处理错误");
}

void test_batch_aos() {
    // 固定 seed 让 N=1 与 N=7 的第一个元素拥有相同输入，
    // 从而同时覆盖单元素、小批量以及元素与系数对应关系。
    const double dt = 0.13;
    const int step1 = 1;
    const int step2 = 13;
    const int number = 7;
    const int number1 = 1;
    const int seed = 13;
    const double time1 = dt * step1;
    const double time2 = dt * (step1 + step2);

    OscillatorBatch oscillator_aos_batch = make_OscillatorAoS_batch(number, dt, seed);
    OscillatorBatch oscillator_aos_batch1 = make_OscillatorAoS_batch(number1, dt, seed);
    OscillatorBatch initial_oscillator_aos_batch = oscillator_aos_batch;
    OscillatorBatch initial_oscillator_aos_batch1 = oscillator_aos_batch1;

    // 一步结果与从初始状态直接求得的 M1 解析参考比较。
    update_aos_batch(oscillator_aos_batch, step1);
    update_aos_batch(oscillator_aos_batch1, step1);
    for (int i = 0; i < number; ++i) {
        OscillatorAoS system_aos = system_to_aos(initial_oscillator_aos_batch[i], time1, dt);
        expect_aos_near(oscillator_aos_batch[i], system_aos,
                        "AoS批量计算欠阻尼振子与单一计算精确解比较");
        if (i == 0) {
            OscillatorAoS system_aos1 = system_to_aos(initial_oscillator_aos_batch1[i], time1, dt);
            expect_aos_near(oscillator_aos_batch1[i], system_aos1,
                            "AoS计算单个欠阻尼振子与单一计算精确解比较");
        };
    }
    // 再从一步后的状态继续推进，验证连续多步的累计时间语义。
    update_aos_batch(oscillator_aos_batch, step2);
    update_aos_batch(oscillator_aos_batch1, step2);
    for (int i = 0; i < number; ++i) {
        OscillatorAoS system_aos = system_to_aos(initial_oscillator_aos_batch[i], time2, dt);
        expect_aos_near(oscillator_aos_batch[i], system_aos,
                        "AoS批量计算欠阻尼振子与单一计算精确解比较");
        if (i == 0) {
            OscillatorAoS system_aos1 = system_to_aos(initial_oscillator_aos_batch1[i], time2, dt);
            expect_aos_near(oscillator_aos_batch1[i], system_aos1,
                            "AoS批量计算欠阻尼振子与单一计算精确解比较");
        }
    }
}

}  // namespace oscillator
