#include <vector>

#include "heat_diffusion.h"
#include "state.h"
#include "test_common.h"

namespace {

const std::size_t nx = 128;
const std::size_t ny = 128;
const double length_x = 1;
const double length_y = 1;
const double dx = length_x / (nx - 1);
const double dy = length_y / (ny - 1);

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
                               const double tolerance) {
    TemperatureField test_temperature_field(test_gird, heat_diffusion::InitialField::SineMode, 16);

    TemperatureField numerical_temperature_field = test_temperature_field;
    TemperatureField analytical_temperature_field = test_temperature_field;

    NuemricalWorkspace workspace =
        numerical_temperature_field.make_numerical_workspace(thermal_diffusivity, dt);
    numerical_temperature_field.numerical_solve(workspace, 1);
    analytical_temperature_field.analytical_solve(thermal_diffusivity, dt);
    const std::vector<double>& numerical_result = numerical_temperature_field.get_temperature();
    const std::vector<double>& analyticall_result = analytical_temperature_field.get_temperature();

    std::size_t node_count = nx * ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_result[i], analyticall_result[i], tolerance);
    }
}

void test_numberical_solve(const double thermal_diffusivity, const int steps, const double dt,
                           const double tolerance) {
    TemperatureField test_temperature_field(test_gird, heat_diffusion::InitialField::SineMode, 16);

    TemperatureField numerical_temperature_field = test_temperature_field;
    TemperatureField analytical_temperature_field = test_temperature_field;

    double time = steps * dt;

    NuemricalWorkspace workspace =
        numerical_temperature_field.make_numerical_workspace(thermal_diffusivity, dt);
    numerical_temperature_field.numerical_solve(workspace, steps);
    analytical_temperature_field.analytical_solve(thermal_diffusivity, time);
    const std::vector<double>& numerical_result = numerical_temperature_field.get_temperature();
    const std::vector<double>& analyticall_result = analytical_temperature_field.get_temperature();

    std::size_t node_count = nx * ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_result[i], analyticall_result[i], tolerance);
    }
}

}  // namespace heat_diffusion
