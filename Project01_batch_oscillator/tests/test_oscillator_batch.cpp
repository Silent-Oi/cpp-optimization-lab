#include <stdexcept>
#include <vector>

#include "oscillator_batch.h"
#include "state.h"
#include "test_common.h"
#include "underdamped_oscillator.h"

namespace oscillator {

// 从批量元素保存的初始状态和原始参数构造 M1 标量解析参考。
// 参考状态直接计算到绝对时间 time，不复用待测的批量更新路径。
static OscillatorAoS system_to_aos(const OscillatorAoS& oscillator, double time, double dt) {
    UnderdampedOscillator system(oscillator.omega, oscillator.zeta);
    State initail_state = {.position = oscillator.position, .velocity = oscillator.velocity};
    StepCoefficients step_cofficients = system.make_step_coefficients(dt);
    State final_state = system.exact_state(time, initail_state);

    return {.position = final_state.position,
            .velocity = final_state.velocity,
            .omega = oscillator.omega,
            .zeta = oscillator.zeta,
            .m00 = step_cofficients.m00,
            .m01 = step_cofficients.m01,
            .m10 = step_cofficients.m10,
            .m11 = step_cofficients.m11};
}

static OscillatorSoABatch system_to_soa(const OscillatorSoABatch& oscillator, double time, double dt) {
    std::size_t N = oscillator.omega.size();

    std::vector<double> position_batch(N);
    std::vector<double> velocity_batch(N);
    std::vector<double> m00_batch(N);
    std::vector<double> m01_batch(N);
    std::vector<double> m10_batch(N);
    std::vector<double> m11_batch(N);
    std::vector<double> omega_batch(N);
    std::vector<double> zeta_batch(N);

    for (std::size_t i = 0; i < N; ++i) {
        UnderdampedOscillator system(oscillator.omega[i], oscillator.zeta[i]);
        State initail_state = {.position = oscillator.position[i],
                               .velocity = oscillator.velocity[i]};
        StepCoefficients step_cofficients = system.make_step_coefficients(dt);
        State final_state = system.exact_state(time, initail_state);

        position_batch[i] = final_state.position;
        velocity_batch[i] = final_state.velocity;
        m00_batch[i] = step_cofficients.m00;
        m01_batch[i] = step_cofficients.m01;
        m10_batch[i] = step_cofficients.m10;
        m11_batch[i] = step_cofficients.m11;
        omega_batch[i] = oscillator.omega[i];
        zeta_batch[i] = oscillator.zeta[i];
    }

    return {.position = position_batch,
            .velocity = velocity_batch,
            .m00 = m00_batch,
            .m01 = m01_batch,
            .m10 = m10_batch,
            .m11 = m11_batch,
            .omega = omega_batch,
            .zeta = zeta_batch};
}

void test_batch_number() {
    // 初始化接口应支持调用者指定的任意非负批量规模。
    const double dt = 0.1;
    const int number = 10;
    const int seed = 12345;
    OscillatorAoSBatch oscillator_aos_batch = make_oscillator_aos_batch(number, dt, seed);
    OscillatorSoABatch oscillator_soa_batch = make_oscillator_soa_batch(number, dt, seed);
    const std::size_t i = number;

    if (oscillator_aos_batch.size() == i && oscillator_soa_batch.omega.size() == i) {
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
    OscillatorAoSBatch oscillator_aos_batch = make_oscillator_aos_batch(number, dt, seed);
    OscillatorSoABatch oscillator_soa_batch = make_oscillator_soa_batch(number, dt, seed);
    update_aos_batch(oscillator_aos_batch, step1);
    update_aos_batch(oscillator_aos_batch, step2);
    update_soa_batch(oscillator_soa_batch, step1);
    update_soa_batch(oscillator_soa_batch, step2);

    if (oscillator_aos_batch.size() == 0 && oscillator_soa_batch.omega.size() == 0) {
        return;
    }
    throw std::runtime_error("空振子处理错误");
}

void test_batch() {
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

    OscillatorAoSBatch oscillator_aos_batch = make_oscillator_aos_batch(number, dt, seed);
    OscillatorAoSBatch oscillator_aos_batch1 = make_oscillator_aos_batch(number1, dt, seed);
    OscillatorSoABatch oscillator_soa_batch = make_oscillator_soa_batch(number, dt, seed);
    OscillatorSoABatch oscillator_soa_batch1 = make_oscillator_soa_batch(number1, dt, seed);
    OscillatorAoSBatch initial_oscillator_aos_batch = oscillator_aos_batch;
    OscillatorAoSBatch initial_oscillator_aos_batch1 = oscillator_aos_batch1;
    OscillatorSoABatch initial_oscillator_soa_batch = oscillator_soa_batch;
    OscillatorSoABatch initial_oscillator_soa_batch1 = oscillator_soa_batch1;

    // 一步结果与从初始状态直接求得的 M1 解析参考比较。
    update_aos_batch(oscillator_aos_batch, step1);
    update_aos_batch(oscillator_aos_batch1, step1);
    update_soa_batch(oscillator_soa_batch, step1);
    update_soa_batch(oscillator_soa_batch1, step1);

    for (int i = 0; i < number; ++i) {
        OscillatorAoS system_aos = system_to_aos(initial_oscillator_aos_batch[i], time1, dt);
        expect_aos_near(oscillator_aos_batch[i], system_aos,
                        "AoS批量单步计算欠阻尼振子与单一计算精确解比较");
        if (i == 0) {
            OscillatorAoS system_aos1 = system_to_aos(initial_oscillator_aos_batch1[i], time1, dt);
            expect_aos_near(oscillator_aos_batch1[i], system_aos1,
                            "AoS单步计算单个欠阻尼振子与单一计算精确解比较");
        };
    }

    OscillatorSoABatch system_soa = system_to_soa(initial_oscillator_soa_batch, time1, dt);
    expect_soa_near(oscillator_soa_batch, system_soa, "SoA批量单步计算欠阻尼振子与单一计算精确解比较");

    OscillatorSoABatch system_soa1 = system_to_soa(initial_oscillator_soa_batch1, time1, dt);
    expect_soa_near(oscillator_soa_batch1, system_soa1, "SoA单步计算单个欠阻尼振子与单一计算精确解比较");


    // 再从一步后的状态继续推进，验证连续多步的累计时间语义。
    update_aos_batch(oscillator_aos_batch, step2);
    update_aos_batch(oscillator_aos_batch1, step2);
    update_soa_batch(oscillator_soa_batch, step2);
    update_soa_batch(oscillator_soa_batch1, step2);

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

    system_soa = system_to_soa(initial_oscillator_soa_batch, time2, dt);
    expect_soa_near(oscillator_soa_batch, system_soa, "SoA批量计算欠阻尼振子与单一计算精确解比较");

    system_soa1 = system_to_soa(initial_oscillator_soa_batch1, time2, dt);
    expect_soa_near(oscillator_soa_batch1, system_soa1,
                    "SoA计算单个欠阻尼振子与单一计算精确解比较");
}


}  // namespace oscillator
