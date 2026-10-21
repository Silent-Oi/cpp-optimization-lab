#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "common_benchmark.h"
#include "heat_diffusion.h"
#include "state.h"

namespace fs = std::filesystem;

namespace {

struct HeatDiffusionConfig {
    heat_diffusion::Grid2D gird;
    heat_diffusion::InitialField initial_field_type;
    double initial_temperature;
    double thermal_diffusivity;
    int steps;
    int experiment_count;
    double dt;
};

heat_diffusion::Grid2D default_grid{
    512,
    512,
    1.0 / 511,
    1.0 / 511,
};

// 默认设置时间步 dt <= 0.09537
HeatDiffusionConfig default_config{
    default_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 1, 9, 0.01,
};

enum class ExperimentType { UnstableTimeStep, GridRefinement, TimeStepRefinement };

struct ErrorMetrics {
    double mean_absolute_error;
    double root_mean_square_error;
};

static ErrorMetrics compute_error_metrics(const heat_diffusion::TemperatureGrid& expected,
                                          const heat_diffusion::TemperatureGrid& actual) {
    if (expected.temperature.size() != actual.temperature.size()) {
        throw std::invalid_argument("预期值与实际值温度场不同");
    }

    std::size_t node_count = actual.temperature.size();

    const double* expected_temperature = expected.temperature.data();
    const double* actual_temperature = actual.temperature.data();

    double error = 0;
    double mae = 0;
    double rmse = 0;

    for (int i = 0; i < node_count; ++i) {
        error = actual_temperature[i] - expected_temperature[i];

        mae += std::abs(error);
        rmse += error * error;
    }

    mae = mae / node_count;
    rmse = std::sqrt(rmse / node_count);

    return {
        mae,
        rmse,
    };
}

}  // namespace

// ============================================================================
// 误差实验
// ============================================================================

namespace error_experiment {

HeatDiffusionConfig unstable_dt_config{
    default_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 1, 9, 0.25,
};

heat_diffusion::Grid2D grid_refinement_grid{
    4,
    4,
    1.0 / 3,
    1.0 / 3,
};

HeatDiffusionConfig grid_refinement_config{
    grid_refinement_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 64, 9, 0.001,
};

static ErrorMetrics benchmark(const int steps, const double dt, const double thermal_diffusivity,
                              const heat_diffusion::TemperatureGrid& initial_temperature_field) {
    double time = dt * steps;

    heat_diffusion::TemperatureGrid numerical_temperature_field = initial_temperature_field;
    heat_diffusion::TemperatureGrid analytical_temperature_field = initial_temperature_field;

    heat_diffusion::numerical::solve(numerical_temperature_field, steps);
    heat_diffusion::analytical::solve(analytical_temperature_field, thermal_diffusivity, time);

    return compute_error_metrics(analytical_temperature_field, numerical_temperature_field);
}

static ErrorMetrics run_mode(HeatDiffusionConfig& experience_config,
                             const ExperimentType& experiment_type,
                             heat_diffusion::TemperatureGrid& initial_temperature_field,
                             std::ofstream& csv) {
    switch (experiment_type) {
        case ExperimentType::UnstableTimeStep: {
            experience_config.steps = experience_config.steps * 2;
            std::cout << std::setprecision(10) << "steps: " << experience_config.steps << '\n';

            csv << ' ' << ',' << ' ' << ',' << ' ' << ',' << ' ' << ',' << experience_config.steps
                << ',';

            return benchmark(experience_config.steps, experience_config.dt,
                             experience_config.thermal_diffusivity, initial_temperature_field);
        }

        case ExperimentType::GridRefinement: {
            experience_config.gird.nx = experience_config.gird.nx * 2;
            experience_config.gird.ny = experience_config.gird.ny * 2;
            experience_config.gird.dx = 1.0 / (experience_config.gird.nx - 1);
            experience_config.gird.dy = 1.0 / (experience_config.gird.ny - 1);
            initial_temperature_field = heat_diffusion::make_temperature_grid(
                experience_config.gird, experience_config.initial_field_type,
                experience_config.initial_temperature, experience_config.thermal_diffusivity,
                experience_config.dt);
            std::cout << std::setprecision(10) << "nodes: " << experience_config.gird.nx << '\n';
            csv << experience_config.gird.nx << ',' << experience_config.gird.ny << ',' << ' '
                << ',' << ' ' << ',' << ' ' << ',';
            return benchmark(experience_config.steps, experience_config.dt,
                             experience_config.thermal_diffusivity, initial_temperature_field);
        }
    }
    throw std::invalid_argument("错误的实验类型");
}

static void run(const std::string& data, const std::string& experiment_name,
                const std::string& filename, const HeatDiffusionConfig& experience_config,
                const ExperimentType& experiment_type) {
    std::ofstream csv =
        common::create_result_csv(PROJECT02_SOURCE_DIR, data, experiment_name, filename);
    csv << "nx,ny,thermal_diffusivity,dt,steps,mae,rmse\n";

    std::cout << std::setprecision(10);
    csv << std::setprecision(10);

    std::size_t experiment_count = experience_config.experiment_count;

    heat_diffusion::TemperatureGrid initial_temperature_field =
        heat_diffusion::make_temperature_grid(
            experience_config.gird, experience_config.initial_field_type,
            experience_config.initial_temperature, experience_config.thermal_diffusivity,
            experience_config.dt);

    csv << experience_config.gird.nx << ',' << experience_config.gird.ny << ','
        << experience_config.thermal_diffusivity << ',' << experience_config.dt << ','
        << experience_config.steps << ' ' << ',' << ' ' << '\n';

    // 输出基础信息
    std::cout << "*******************     BENTCHMARK     *********************" << '\n';
    std::cout << "dx: " << experience_config.gird.dx << '\n';
    std::cout << "dy: " << experience_config.gird.dy << '\n';
    std::cout << "thermal_diffusivity: " << experience_config.thermal_diffusivity << '\n';
    std::cout << "dt: " << experience_config.dt << '\n';
    std::cout << "steps: " << experience_config.steps << '\n';
    std::cout << "*******************          *********************" << '\n';

    HeatDiffusionConfig bench_config = experience_config;

    for (int i = 0; i < experiment_count; ++i) {
        ErrorMetrics result =
            run_mode(bench_config, experiment_type, initial_temperature_field, csv);

        std::cout << "mae: " << result.mean_absolute_error << '\n';
        std::cout << "rmse: " << result.root_mean_square_error << '\n';
        std::cout << '\n';
        csv << result.mean_absolute_error << ',' << result.root_mean_square_error << '\n';
    }
}

}  // namespace error_experiment

int main() {
    auto data = common::make_run_timestamp();
    std::string experiment_name = "error_experiment";
    std::string filename = "grid_refinement_experiment";

    const ExperimentType experiment_type = ExperimentType::GridRefinement;

    error_experiment::run(data, experiment_name, filename, error_experiment::grid_refinement_config,
                          experiment_type);
}
