#include "heat_diffusion.h"

#include <cmath>
#include <numbers>
#include <vector>

#include "state.h"

namespace {

static heat_diffusion::TemperatureGrid initialize_uniform_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature) {
    std::size_t node_count = grid.nx * grid.ny;

    return {
        grid,
        std::vector<double>(node_count, initial_temperature),
    };
}

static double sine_mode_temperature(const heat_diffusion::Grid2D& grid, double initial_temperature,
                                    double x, double y) {
    return initial_temperature * std::sin(std::numbers::pi * x / (grid.dx * (grid.nx - 1))) *
           std::sin(std::numbers::pi * y / (grid.dy * (grid.ny - 1)));
}

static heat_diffusion::TemperatureGrid initialize_SineMode_temperature(
    const heat_diffusion::Grid2D& grid, double initial_temperature) {
    std::size_t node_count = grid.nx * grid.ny;
    std::vector<double> temperature(node_count);

    std::size_t number_y = grid.ny;
    std::size_t number_x = grid.nx;

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            double x = i * grid.dx;
            double y = j * grid.dy;
            std::size_t index = j * grid.nx + i;
            temperature[index] = sine_mode_temperature(grid, initial_temperature, x, y);
        }
    }
    return {
        grid,
        temperature,
    };
}

}  // namespace

namespace heat_diffusion {

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

namespace numerical {}

namespace analytical {

void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double time) {
    std::size_t number_y = temperature_field.grid.ny;
    std::size_t number_x = temperature_field.grid.nx;
    double diffusivity = std::exp(-1 * thermal_diffusivity * time);

    for (std::size_t j = 0; j < number_y; ++j) {
        for (std::size_t i = 0; i < number_x; ++i) {
            std::size_t index = j * temperature_field.grid.nx + i;
            temperature_field.temperature[index] =
                diffusivity * temperature_field.temperature[index];
        }
    }
}

}  // namespace numerical

}  // namespace heat_diffusion
