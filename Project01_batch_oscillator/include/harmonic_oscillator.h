#pragma once
#include "state.h"

namespace oscillator
{
   // 归一化的一维简谐振子，满足 x'' = -omega^2 * x。
   // 该类只保存系统参数，不保存随时间变化的状态。
   class HarmonicOscillator {
   public:
       // angular_frequency 必须有限且严格大于零。
       explicit HarmonicOscillator(double angular_frequency);

       // 返回角频率 omega，单位为弧度/单位时间。
       double angular_frequency() const;

       // 计算单位质量机械能：0.5 * (v^2 + omega^2 * x^2)。
       double system_energy(const State& state) const;

       // 根据 t=0 时的 initial_state 计算给定时刻的解析状态。
       State exact_state(double time, const State& initial_state) const;

       // 计算 state 对应的 (dx/dt, dv/dt)，不推进时间。
       StateDerivative derivative(const State& state) const;

    private:
       double angular_frequency_;
   };

}
