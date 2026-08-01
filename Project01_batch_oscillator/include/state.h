#pragma once
#include <cmath>

namespace oscillator {
// 单个振子在同一物理时刻的位置和速度。
struct State {
    double position;
    double velocity;

    // 使用固定绝对容差比较两个状态。
    bool operator==(const State& other) const {
        constexpr double eps = 1e-9;
        return std::abs(position - other.position) < eps &&
               std::abs(velocity - other.velocity) < eps;
    }
};

// State 的时间导数：(dx/dt, dv/dt)。
// 使用独立类型，避免把变化率误当成实际状态值。
struct StateDerivative {
    double position_rate;
    double velocity_rate;

    // 使用固定绝对容差逐分量比较两个状态导数。
    bool operator==(const StateDerivative& other) const {
        constexpr double eps = 1e-9;
        return std::abs(position_rate - other.position_rate) < eps &&
               std::abs(velocity_rate - other.velocity_rate) < eps;
    }
};
// Position Verlet 所需的两个连续位置。
// current_position 表示 x_n，previous_position 表示 x_{n-1}。
struct PositionVerletState {
    double current_position;
    double previous_position;
};

// 欠阻尼振子的精确一步状态转移矩阵：
// [x_{n+1}, v_{n+1}]^T = M * [x_n, v_n]^T。
// 转矩矩阵只和dt以及振子属性有关。
struct StepCoefficients {
    double m00;
    double m01;
    double m10;
    double m11;
};

// AoS元素：一个对象完整保存一个振子的全部数据。
// position、velocity 是每步更新的状态；omega、zeta 是初始化后不变的原始参数；
// m00～m11 是由原始参数和固定 dt 预计算得到的更新系数。
struct OscillatorAoS {
    double position;
    double velocity;

    double omega;
    double zeta;

    double m00;
    double m01;
    double m10;
    double m11;
};

// 对一次批量运行结果的紧凑摘要，用于固定输入运行和回归检查。
struct AoSResults {
    std::size_t N;
    double state_checksum;
    double max_abs_x;
    double max_abs_v;
    bool finite;

    bool operator==(const AoSResults& other) const {
        constexpr double eps = 1e-9;
        return N == other.N && std::abs(state_checksum - other.state_checksum) < eps &&
               std::abs(max_abs_x - other.max_abs_x) < eps &&
               std::abs(max_abs_v - other.max_abs_v) < eps && finite == other.finite;
    }
};

}  // namespace oscillator
