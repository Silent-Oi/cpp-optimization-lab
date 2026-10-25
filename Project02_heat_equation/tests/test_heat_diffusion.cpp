#include <vector>

#include "heat_diffusion.h"
#include "state.h"
#include "test_common.h"

namespace {

heat_diffusion::Grid2D test_gird{
    64,
    64,
    1.0 / 63,
    1.0 / 63,
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

    std::size_t node_count = test_gird.nx * test_gird.ny;

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

    std::size_t node_count = test_gird.nx * test_gird.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_result[i], analyticall_result[i], tolerance);
    }
}

void test_numberical_solve_parallel(const double thermal_diffusivity, const int steps,
                                    const double dt, const std::size_t thread_cout,
                                    const double tolerance) {
    TemperatureField test_field(test_gird, heat_diffusion::InitialField::SineMode, 16);
    TemperatureField numerical_field = test_field;
    TemperatureField parallel_field = test_field;

    NuemricalWorkspace workspace = test_field.make_numerical_workspace(thermal_diffusivity, dt);

    numerical_field.numerical_solve(workspace, steps);
    parallel_field.numerical_solve_parallel(workspace, steps, thread_cout);

    const std::vector<double>& numerical_result = numerical_field.get_temperature();
    const std::vector<double>& parallel_result = parallel_field.get_temperature();

    std::size_t node_count = test_gird.nx * test_gird.ny;

    for (std::size_t i = 0; i < node_count; ++i) {
        common::expect_near(numerical_result[i], parallel_result[i], tolerance);
    }

}

}  // namespace heat_diffusion
