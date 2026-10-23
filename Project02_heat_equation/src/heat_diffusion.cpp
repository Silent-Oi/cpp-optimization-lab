#include "heat_diffusion.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>
#include <vector>

#include "state.h"

namespace {

// ---- 初始均匀温度场 --------------------------------------------------------------

static std::vector<double> initialize_uniform_temperature(const heat_diffusion::Grid2D& grid,
                                                          double initial_temperature) {
    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;
    std::size_t node_count = number_y * number_x;
    std::vector<double> temperature(node_count, initial_temperature);

    for (std::size_t i = 0; i < number_x; ++i) {
        temperature[i] = 0;
        temperature[(number_y - 1) * number_x + i] = 0;
    }

    for (std::size_t j = 1; j < number_y - 1; ++j) {
        const std::size_t row = j * number_x;
        temperature[row] = 0;
        temperature[row + number_x - 1] = 0;
    }

    return temperature;
}

// ---- 初始正弦温度场 --------------------------------------------------------------

static std::vector<double> initialize_SineMode_temperature(const heat_diffusion::Grid2D& grid,
                                                           double initial_temperature) {
    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;
    std::size_t node_count = number_y * number_x;
    std::vector<double> temperature(node_count);

    auto sine_mode_temperature = [&grid, initial_temperature](const double x, const double y) {
        return initial_temperature * std::sin(std::numbers::pi * x / (grid.dx * (grid.nx - 1))) *
               std::sin(std::numbers::pi * y / (grid.dy * (grid.ny - 1)));
    };

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            double x = i * grid.dx;
            double y = j * grid.dy;
            std::size_t index = j * grid.nx + i;

            if (j == 0 || i == 0 || j == number_y - 1 || i == number_x - 1) {
                temperature[index] = 0;
                continue;
            }
            temperature[index] = sine_mode_temperature(x, y);
        }
    }
    return temperature;
}

}  // namespace

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

TemperatureField::TemperatureField(const Grid2D grid, InitialField initial_field_type,
                                   double initial_temperature)
    : grid_{grid} {
    if (grid.nx < 3 || grid.ny < 3 || grid.dx <= 0 || grid.dy <= 0 || !std::isfinite(grid.dx) ||
        !std::isfinite(grid.dy)) {
        throw std::invalid_argument("网格有问题");
    }
    switch (initial_field_type) {
        case InitialField::Uniform:
            temperature_ = initialize_uniform_temperature(grid, initial_temperature);
            return;
        case InitialField::SineMode:
            temperature_ = initialize_SineMode_temperature(grid, initial_temperature);
            return;
    };

    throw std::invalid_argument("错误的初始场类型");
}

// ---- 获取场信息 --------------------------------------------------------------

const Grid2D& TemperatureField::get_grid() const {
    return grid_;
}

const std::vector<double>& TemperatureField::get_temperature() const {
    return temperature_;
}

// ---- 设置场信息 --------------------------------------------------------------



// ---- 预计算更新缓存 --------------------------------------------------------------

const NuemricalWorkspace TemperatureField::make_numerical_workspace(double thermal_diffusivity,
                                                                    double dt) const {
    double rx = thermal_diffusivity * dt / (grid_.dx * grid_.dx);
    double ry = thermal_diffusivity * dt / (grid_.dy * grid_.dy);
    double rc = 1 - 2 * rx - 2 * ry;
    std::vector<double> temperature_cache = temperature_;
    return {
        rx,
        ry,
        rc,
        std::move(temperature_cache),
    };
}

// ============================================================================
// 中心差分解
// ============================================================================

const void TemperatureField::numerical_solve_step(NuemricalWorkspace& workspace) {
    const std::size_t number_y = grid_.ny;
    const std::size_t number_x = grid_.nx;
    const double rx = workspace.rx;
    const double ry = workspace.ry;
    const double rc = workspace.rc;
    const double* __restrict old_temperature = temperature_.data();
    double* __restrict new_temperature = workspace.temperature_cache.data();

    for (std::size_t j = 1; j < number_y - 1; ++j) {
        for (std::size_t i = 1; i < number_x - 1; ++i) {
            std::size_t index = j * number_x + i;

            double orign_temperature = old_temperature[index];
            double left_temperature = old_temperature[index - 1];
            double right_temperature = old_temperature[index + 1];
            double down_temperature = old_temperature[index - number_x];
            double up_temperature = old_temperature[index + number_x];

            new_temperature[index] = rc * orign_temperature +
                                     rx * (right_temperature + left_temperature) +
                                     ry * (up_temperature + down_temperature);
        }
    }
    temperature_.swap(workspace.temperature_cache);
}

const void TemperatureField::numerical_solve(NuemricalWorkspace& workspace, const int steps) {
    if (steps < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    for (std::size_t i = 0; i < steps; ++i) {
        numerical_solve_step(workspace);
    }
}

// ============================================================================
// 解析解(暂时只支持正弦场）
// ============================================================================

const void TemperatureField::analytical_solve(const double thermal_diffusivity, const double time) {
    std::size_t number_y = grid_.ny;
    std::size_t number_x = grid_.nx;
    std::size_t node_count = number_y * number_x;
    double length_x = grid_.dx * (grid_.nx - 1);
    double length_y = grid_.dy * (grid_.ny - 1);

    double lambda = thermal_diffusivity * std::numbers::pi * std::numbers::pi *
                    (1 / (length_x * length_x) + 1 / (length_y * length_y));
    double diffusivity = std::exp(-1 * lambda * time);

    for (std::size_t i = 0; i < node_count; ++i) {
        temperature_[i] = diffusivity * temperature_[i];
    }
}

}  // namespace heat_diffusion
