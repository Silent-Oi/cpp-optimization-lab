#include <vector>

#include "heat_diffusion.h"
#include "state.h"
#include "test_common.h"

namespace {

const std::size_t nx = 128;
const std::size_t ny = 128;
const double length_x = 1;
const double length_y = 1;
const double dx = length_x / nx;
const double dy = length_y / ny;

heat_diffusion::Grid2D test_gird{
    nx,
    ny,
    dx,
    dy,
};

}  // namespace

namespace heat_diffusion {

// 验证差分解和解析解差距
// ============================================================================

void test_numerical_solve_step(const double thermal_diffusivity, const double dt,
                               double tolerance) {
    const TemperatureGrid test_temperature_field = make_temperature_grid(
        test_gird, heat_diffusion::InitialField::SineMode, 16, thermal_diffusivity, dt);

    TemperatureGrid numerical_temperature_field = test_temperature_field;
    TemperatureGrid analytical_temperature_field = test_temperature_field;

    numerical::solve(numerical_temperature_field, 1);
    analytical::solve(analytical_temperature_field, thermal_diffusivity, dt);

    std::size_t node_count = test_temperature_field.grid.nx * test_temperature_field.grid.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_temperature_field.temperature[i],
                            analytical_temperature_field.temperature[i], tolerance);
    }
}

void test_numberical_solve(const double thermal_diffusivity, int steps, double dt,
                           double tolerance) {
    const TemperatureGrid test_temperature_field = make_temperature_grid(
        test_gird, heat_diffusion::InitialField::SineMode, 16, thermal_diffusivity, dt);

    TemperatureGrid numerical_temperature_field = test_temperature_field;
    TemperatureGrid analytical_temperature_field = test_temperature_field;
    double time = steps * dt;

    numerical::solve(numerical_temperature_field, steps);
    analytical::solve(analytical_temperature_field, thermal_diffusivity, time);

    std::size_t node_count = test_temperature_field.grid.nx * test_temperature_field.grid.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_temperature_field.temperature[i],
                            analytical_temperature_field.temperature[i], tolerance);
    }
}

}  // namespace heat_diffusion
