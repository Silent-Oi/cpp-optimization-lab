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
    default_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 64, 12, 0.01,
};

enum class ExperimentType { UnstableTimeStep, GridRefinement, TimeStepRefinement };

struct ErrorMetrics {
    double mean_absolute_error;
    double root_mean_square_error;
};

static ErrorMetrics compute_error_metrics(const std::vector<double>& expected,
                                          const std::vector<double>& actual) {
    if (expected.size() != actual.size()) {
        throw std::invalid_argument("预期值与实际值温度场不同");
    }

    std::size_t node_count = actual.size();

    const double* expected_temperature = expected.data();
    const double* actual_temperature = actual.data();

    double error = 0;
    double mae = 0;
    double rmse = 0;

    for (std::size_t i = 0; i < node_count; ++i) {
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

// ---- 稳定性要求实验设置 --------------------------------------------------------------

HeatDiffusionConfig unstable_dt_config{
    default_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 1, 9, 0.25,
};

// ---- 网格缩放实验设置 --------------------------------------------------------------

heat_diffusion::Grid2D grid_refinement_grid{
    5,
    5,
    1.0 / 4,
    1.0 / 4,
};

HeatDiffusionConfig grid_refinement_config{
    grid_refinement_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 16, 11, 0.000001,
};

// ---- 时间步缩放实验设置 --------------------------------------------------------------

heat_diffusion::Grid2D time_step_refinement_grid{
    1025,
    1025,
    1.0 / 1024,
    1.0 / 1024,
};

HeatDiffusionConfig time_step_refinement_config{
    time_step_refinement_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 64, 8, 0.01,
};

// ---- 误差计算 --------------------------------------------------------------

static ErrorMetrics benchmark(const int steps, const double dt, const double thermal_diffusivity,
                              const heat_diffusion::TemperatureField& initial_temperature_field) {
    double time = dt * steps;

    heat_diffusion::TemperatureField numerical_temperature_field = initial_temperature_field;
    heat_diffusion::TemperatureField analytical_temperature_field = initial_temperature_field;

    heat_diffusion::NuemricalWorkspace workspace =
        numerical_temperature_field.make_numerical_workspace(thermal_diffusivity, dt);
    numerical_temperature_field.numerical_solve(workspace, steps);
    analytical_temperature_field.analytical_solve(thermal_diffusivity, time);
    const std::vector<double>& numerical_result = numerical_temperature_field.get_temperature();
    const std::vector<double>& analyticall_result = analytical_temperature_field.get_temperature();

    return compute_error_metrics(analyticall_result, numerical_result);
}

// ---- 实验类型选择 --------------------------------------------------------------

static ErrorMetrics run_mode(HeatDiffusionConfig& benchmark_config,
                             const ExperimentType& experiment_type,
                             heat_diffusion::TemperatureField& initial_temperature_field,
                             std::ofstream& csv) {
    switch (experiment_type) {
        case ExperimentType::UnstableTimeStep: {
            benchmark_config.steps = benchmark_config.steps * 2;
            std::cout << "steps: " << benchmark_config.steps << '\n';

            csv << ' ' << ',' << ' ' << ',' << ' ' << ',' << ' ' << ',' << benchmark_config.steps
                << ',';

            return benchmark(benchmark_config.steps, benchmark_config.dt,
                             benchmark_config.thermal_diffusivity, initial_temperature_field);
        }

        case ExperimentType::GridRefinement: {
            benchmark_config.gird.nx = benchmark_config.gird.nx * 2 - 1;
            benchmark_config.gird.ny = benchmark_config.gird.ny * 2 - 1;
            benchmark_config.gird.dx = 1.0 / (benchmark_config.gird.nx - 1);
            benchmark_config.gird.dy = 1.0 / (benchmark_config.gird.ny - 1);
            initial_temperature_field = heat_diffusion::TemperatureField(
                benchmark_config.gird, benchmark_config.initial_field_type,
                benchmark_config.initial_temperature);
            ;

            std::cout << "nodes: " << benchmark_config.gird.nx << '\n';
            csv << benchmark_config.gird.nx << ',' << benchmark_config.gird.ny << ',' << ' ' << ','
                << ' ' << ',' << ' ' << ',';
            return benchmark(benchmark_config.steps, benchmark_config.dt,
                             benchmark_config.thermal_diffusivity, initial_temperature_field);
        }

        case ExperimentType::TimeStepRefinement: {
            benchmark_config.dt = benchmark_config.dt * 0.5;
            benchmark_config.steps = benchmark_config.steps * 2;
            std::cout << "dt: " << benchmark_config.dt << '\n';
            csv << ' ' << ',' << ' ' << ',' << ' ' << ',' << benchmark_config.dt << ','
                << benchmark_config.steps << ',';
            return benchmark(benchmark_config.steps, benchmark_config.dt,
                             benchmark_config.thermal_diffusivity, initial_temperature_field);
        }
    }
    throw std::invalid_argument("错误的实验类型");
}

// ---- 实验主体 --------------------------------------------------------------

static void run(const std::string& data, const std::string& experiment_name,
                const std::string& filename, const HeatDiffusionConfig& experience_config,
                const ExperimentType experiment_type) {
    if (experience_config.initial_field_type != heat_diffusion::InitialField::SineMode) {
        throw std::invalid_argument("暂不支持其他场");
    }

    std::ofstream csv =
        common::create_result_csv(PROJECT02_SOURCE_DIR, data, experiment_name, filename);
    csv << "nx,ny,thermal_diffusivity,dt,steps,mae,rmse\n";

    std::cout << std::setprecision(10);
    csv << std::setprecision(10);

    std::size_t experiment_count = experience_config.experiment_count;

    heat_diffusion::TemperatureField initial_temperature_field(
        experience_config.gird, experience_config.initial_field_type,
        experience_config.initial_temperature);

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

    HeatDiffusionConfig benchmark_config = experience_config;

    for (int i = 0; i < experiment_count; ++i) {
        ErrorMetrics result =
            run_mode(benchmark_config, experiment_type, initial_temperature_field, csv);

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
    std::string filename = "time_step_refinement_experiment";

    const ExperimentType experiment_type = ExperimentType::TimeStepRefinement;

    error_experiment::run(data, experiment_name, filename, error_experiment::time_step_refinement_config,
                          experiment_type);
}
