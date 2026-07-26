#pragma once
#include <cmath>

namespace oscillator {
// 单个振子在同一物理时刻的位置和速度。
struct State {
    double position;
    double velocity;

    // 使用固定绝对容差比较两个状态，供当前验证代码判断近似相等。
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
// 固定系统参数和 dt 时，这四个系数可在时间循环外预计算并重复使用。
struct StepCoefficients {
    double m00;
    double m01;
    double m10;
    double m11;
};

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

struct AoSResults {
    std::size_t N;
    double state_checksum;
    double max_abs_x;
    double max_abs_v;
    bool finite;
};


}
