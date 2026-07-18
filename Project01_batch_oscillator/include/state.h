#pragma once

namespace oscillator {
// 单个振子在同一物理时刻的位置和速度。
struct State {
    double position;
    double velocity;
};

// State 的时间导数：(dx/dt, dv/dt)。
// 使用独立类型，避免把变化率误当成实际状态值。
struct StateDerivative {
    double position_rate;
    double velocity_rate;
};

// Position Verlet 所需的两个连续位置。
// current_position 表示 x_n，previous_position 表示 x_{n-1}。
struct PositionVerletState {
    double current_position;
    double previous_position;
};

}
