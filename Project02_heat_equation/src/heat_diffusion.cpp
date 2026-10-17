#include "heat_diffusion.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <vector>

#include "state.h"

namespace {

// ---- 初始均匀温度场 --------------------------------------------------------------

static heat_diffusion::TemperatureGrid initialize_uniform_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature) {
    std::size_t node_count = grid.nx * grid.ny;

    return {
        grid,
        std::vector<double>(node_count, initial_temperature),
    };
}

// ---- 初始正弦温度场 --------------------------------------------------------------

static heat_diffusion::TemperatureGrid initialize_SineMode_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature) {
    std::size_t node_count = grid.nx * grid.ny;
    std::vector<double> temperature(node_count);

    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;

    auto sine_mode_temperature = [&grid, initial_temperature](const double x, const double y) {
        return initial_temperature * std::sin(std::numbers::pi * x / (grid.dx * (grid.nx - 1))) *
               std::sin(std::numbers::pi * y / (grid.dy * (grid.ny - 1)));
    };

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            double x = i * grid.dx;
            double y = j * grid.dy;
            std::size_t index = j * grid.nx + i;
            temperature[index] = sine_mode_temperature(x, y);
        }
    }
    return {
        grid,
        temperature,
    };
}

}  // namespace

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

TemperatureGrid make_temperature_grid(const Grid2D& grid, InitialField initial_field_type,
                                      double initial_temperature) {
    switch (initial_field_type) {
        case InitialField::Uniform:
            return initialize_uniform_temperature(grid, initial_temperature);
            break;
        case InitialField::SineMode:
            return initialize_SineMode_temperature(grid, initial_temperature);
            break;
    };
}

// ============================================================================
// 中心差分解
// ============================================================================

namespace numerical {

void solve_step(TemperatureGrid& temperature_field, double thermal_diffusivity, double dt) {
    std::size_t number_y = temperature_field.grid.ny;
    std::size_t number_x = temperature_field.grid.nx;

    std::vector<double> temperature_cache = temperature_field.temperature;

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            std::size_t index = j * temperature_field.grid.nx + i;

            if (i == 0 || i == number_x - 1 || j == 0 || j == number_y - 1) {
                temperature_cache[index] = 0;
                continue;
            }

            double orign_temperature = temperature_field.temperature[index];
            double left_temperature = temperature_field.temperature[index - 1];
            double right_temperature = temperature_field.temperature[index + 1];
            double down_temperature = temperature_field.temperature[index - number_x];
            double up_temperature = temperature_field.temperature[index + number_x];

            double ddTx = (right_temperature + left_temperature - 2 * orign_temperature) /
                          (temperature_field.grid.dx * temperature_field.grid.dx);

            double ddTy = (up_temperature + down_temperature - 2 * orign_temperature) /
                          (temperature_field.grid.dy * temperature_field.grid.dy);

            temperature_cache[index] = orign_temperature + thermal_diffusivity * dt * (ddTx + ddTy);
        }
    }

    temperature_field.temperature = temperature_cache;
}

void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double steps,
           double dt) {
    if (steps < 0) {
        throw std::invalid_argument("step must be non-negative");
    }
    if (dt < 0) {
        throw std::invalid_argument("The central difference operator is not invertible.");
    }

    for (std::size_t i = 0; i < steps; ++i) {
        solve_step(temperature_field, thermal_diffusivity, dt);
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
