#include "heat_diffusion.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>

#include "state.h"

namespace {

// ---- 初始均匀温度场 --------------------------------------------------------------

static heat_diffusion::TemperatureGrid initialize_uniform_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature, double thermal_diffusivity,
    double dt) {
    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;
    std::size_t node_count = number_y * number_x;
    std::vector<double> temperature(node_count, initial_temperature);

    double rx = thermal_diffusivity * dt / (grid.dx * grid.dx);
    double ry = thermal_diffusivity * dt / (grid.dy * grid.dy);

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            double x = i * grid.dx;
            double y = j * grid.dy;
            std::size_t index = j * grid.nx + i;

            if (j == 0 || i == 0 || j == number_y - 1 || i == number_x - 1) {
                temperature[index] = 0;
                continue;
            }
        }
    }
    return {
        grid,
        rx,
        ry,
        temperature,
    };
}

// ---- 初始正弦温度场 --------------------------------------------------------------

static heat_diffusion::TemperatureGrid initialize_SineMode_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature, double thermal_diffusivity,
    double dt) {
    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;
    std::size_t node_count = number_y * number_x;
    std::vector<double> temperature(node_count);

    double rx = thermal_diffusivity * dt / (grid.dx * grid.dx);
    double ry = thermal_diffusivity * dt / (grid.dy * grid.dy);

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
    return {
        grid,
        rx,
        ry,
        temperature,
    };
}

}  // namespace

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

TemperatureGrid make_temperature_grid(const Grid2D& grid, InitialField initial_field_type,
                                      double initial_temperature, double thermal_diffusivity,
                                      double dt) {
    if (grid.nx < 3 || grid.ny < 3 || grid.dx <= 0 || grid.dy <= 0 || !std::isfinite(grid.dx) ||
        !std::isfinite(grid.dy)) {
        throw std::invalid_argument("网格有问题");
    }

    if (thermal_diffusivity < 0 || !std::isfinite(thermal_diffusivity) || dt < 0 ||
        !std::isfinite(dt) || !std::isfinite(initial_temperature)) {
        throw std::invalid_argument("初始参数有问题");
    }

    switch (initial_field_type) {
        case InitialField::Uniform:
            return initialize_uniform_temperature(grid, initial_temperature, thermal_diffusivity,
                                                  dt);
            break;
        case InitialField::SineMode:
            return initialize_SineMode_temperature(grid, initial_temperature, thermal_diffusivity,
                                                   dt);
            break;
    };

    throw std::invalid_argument("错误的初始场类型");
}

// ============================================================================
// 中心差分解
// ============================================================================

namespace numerical {

void solve_step(TemperatureGrid& temperature_field, std::vector<double>& temperature_cache) {
    const std::size_t number_y = temperature_field.grid.ny;
    const std::size_t number_x = temperature_field.grid.nx;

    for (std::size_t j = 1; j < number_y - 1; ++j) {
        for (std::size_t i = 1; i < number_x - 1; ++i) {
            std::size_t index = j * number_x + i;

            double orign_temperature = temperature_field.temperature[index];
            double left_temperature = temperature_field.temperature[index - 1];
            double right_temperature = temperature_field.temperature[index + 1];
            double down_temperature = temperature_field.temperature[index - number_x];
            double up_temperature = temperature_field.temperature[index + number_x];

            temperature_cache[index] =
                orign_temperature +
                temperature_field.rx *
                    (right_temperature + left_temperature - 2 * orign_temperature) +
                temperature_field.ry * (up_temperature + down_temperature - 2 * orign_temperature);
        }
    }

    temperature_field.temperature.swap(temperature_cache);
}

void solve(TemperatureGrid& temperature_field, int steps) {
    if (steps < 0) {
        throw std::invalid_argument("step must be non-negative");
    }

    std::size_t node_count = temperature_field.grid.nx * temperature_field.grid.ny;
    std::vector<double> temperature_cache(node_count);

    for (std::size_t i = 0; i < steps; ++i) {
        solve_step(temperature_field, temperature_cache);
    }
}

}  // namespace numerical

// ============================================================================
// 正弦初始场解析解
// ============================================================================

namespace analytical {

void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double time) {
    std::size_t number_y = temperature_field.grid.ny;
    std::size_t number_x = temperature_field.grid.nx;
    double length_x = temperature_field.grid.dx * (temperature_field.grid.nx - 1);
    double length_y = temperature_field.grid.dy * (temperature_field.grid.ny - 1);

    double lambda = thermal_diffusivity * std::numbers::pi * std::numbers::pi *
                    (1 / (length_x * length_x) + 1 / (length_y * length_y));
    double diffusivity = std::exp(-1 * lambda * time);

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            std::size_t index = j * temperature_field.grid.nx + i;
            temperature_field.temperature[index] =
                diffusivity * temperature_field.temperature[index];
        }
    }
}

}  // namespace analytical

}  // namespace heat_diffusion
