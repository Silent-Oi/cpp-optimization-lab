#include <vector>

#include "heat_diffusion.h"
#include "state.h"
#include "test_common.h"

namespace {

const double thermal_diffusivity = 0.001;

heat_diffusion::Grid2D test_gird{
    128,
    128,
    0.015625,
    0.015625,
};

const heat_diffusion::TemperatureGrid test_temperature_field =
    heat_diffusion::make_temperature_grid(test_gird, heat_diffusion::InitialField::SineMode, 16);

}  // namespace

namespace heat_diffusion {

// 验证差分解和解析解差距
// ============================================================================

void test_numerical_solve_step(double dt) {

    TemperatureGrid numerical_temperature_field = test_temperature_field;
    TemperatureGrid analytical_temperature_field = test_temperature_field;

    numerical::solve_step(numerical_temperature_field, thermal_diffusivity, dt);
    analytical::solve(analytical_temperature_field, thermal_diffusivity, dt);

    std::size_t node_count = test_temperature_field.grid.nx * test_temperature_field.grid.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_temperature_field.temperature[i],
                            analytical_temperature_field.temperature[i]);
    }
}

void test_numberical_solve(double steps, double dt) {

    TemperatureGrid numerical_temperature_field = test_temperature_field;
    TemperatureGrid analytical_temperature_field = test_temperature_field;

    numerical::solve(numerical_temperature_field, thermal_diffusivity, steps, dt);
    analytical::solve(analytical_temperature_field, thermal_diffusivity, dt);

    std::size_t node_count = test_temperature_field.grid.nx * test_temperature_field.grid.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_temperature_field.temperature[i],
                            analytical_temperature_field.temperature[i]);
    }



}


}  // namespace heat_diffusion
