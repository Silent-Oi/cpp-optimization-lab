#pragma once
#include "state.h"

namespace oscillator
{
    // 一维欠阻尼振子，满足 x'' = -2*zeta*omega*x' - omega^2*x。
    // 该类只保存不随时间变化的系统参数，不保存振子的当前状态。
    class UnderdampedOscillator {

        public:
        // angular_frequency 必须有限且严格大于零；damping 表示阻尼比 zeta，
        // 当前实现支持 0 <= zeta < 1。
        explicit UnderdampedOscillator(double angular_frequency, double damping);

        // 返回无阻尼固有角频率 omega，单位为弧度/单位时间。
        double angular_frequency() const;
        // 返回无量纲阻尼比 zeta。
        double damping() const;

        // 计算单位质量机械能：0.5 * (v^2 + omega^2 * x^2)。
        double system_energy(const State& state) const;
        // 先计算 initial_state 演化到指定时刻的解析状态，再返回其机械能。
        double system_energy(double time, const State& initial_state) const;

        // 为固定 dt 预计算精确一步状态转移矩阵；时间循环中可重复使用返回值。
        StepCoefficients make_step_coefficients(double dt) const;

        // 根据 t=0 时的 initial_state，直接计算任意给定时刻的解析状态。
        State exact_state(double time, const State& initial_state) const;
        // 将预计算的状态转移矩阵作用于 current_state，得到下一时间步状态。
        State exact_state_step(const StepCoefficients& step_coefficients,
                               const State& current_state) const;

        private:
        // 用户输入的固有角频率 omega 和阻尼比 zeta。
        double angular_frequency_;
        double damping_;

        // 欠阻尼角频率 omega_d = omega*sqrt(1-zeta^2)。
        double omega_d_;
        // 指数衰减率 beta = zeta*omega。
        double beta_;
    };
}
