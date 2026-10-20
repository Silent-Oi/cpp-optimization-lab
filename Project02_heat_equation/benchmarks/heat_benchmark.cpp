#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
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
    int initail_steps;
    int experiment_count;
    double dt;
};

heat_diffusion::Grid2D default_grid{
    512,
    512,
    1.0 / 512,
    1.0 / 512,
};

// 默认设置时间步 dt <= 0.09537
HeatDiffusionConfig default_config{
    default_grid, heat_diffusion::InitialField::SineMode, 128, 1e-5, 1, 9, 0.01,
};

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

    std::vector<double> expected_temperature = expected.temperature;
    std::vector<double> actual_temperature = actual.temperature;

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

static void benchmark(const std::string& data, const std::string& experiment_name,
                      const std::string& filename, const HeatDiffusionConfig& experience_config) {
    std::ofstream csv =
        common::create_result_csv(PROJECT02_SOURCE_DIR, data, experiment_name, filename);
    csv << "dx,dy,thermal_diffusivity,dt,steps,mae,rmse\n";

    std::size_t experiment_count = experience_config.experiment_count;
    double thermal_diffusivity = experience_config.thermal_diffusivity;
    int steps = experience_config.initail_steps;
    double dt = experience_config.dt;
    double time = 0;

    const heat_diffusion::TemperatureGrid initial_temperature_field =
        heat_diffusion::make_temperature_grid(
            experience_config.gird, experience_config.initial_field_type,
            experience_config.initial_temperature, thermal_diffusivity, dt);
    heat_diffusion::TemperatureGrid numerical_temperature_field = initial_temperature_field;
    heat_diffusion::TemperatureGrid analytical_temperature_field = initial_temperature_field;

    csv << experience_config.gird.dx << ',' << experience_config.gird.dy << ','
        << thermal_diffusivity << ',' << dt << ',' << 1 << ' ' << ',' << ' ' << '\n';

    // 输出基础信息
    std::cout << std::setprecision(10)
              << "*******************     BENTCHMARK     *********************" << '\n';
    std::cout << std::setprecision(10) << "dx: " << experience_config.gird.dx << '\n';
    std::cout << std::setprecision(10) << "dy: " << experience_config.gird.dy << '\n';
    std::cout << std::setprecision(10) << "thermal_diffusivity: "
              << experience_config.thermal_diffusivity << '\n';
    std::cout << std::setprecision(10) << "dt: " << dt << '\n';

    for (int i = 0; i < experiment_count; ++i) {
        steps = steps * 2;
        time = dt * steps;

        numerical_temperature_field = initial_temperature_field;
        analytical_temperature_field = initial_temperature_field;

        heat_diffusion::numerical::solve(numerical_temperature_field, steps);
        heat_diffusion::analytical::solve(analytical_temperature_field, thermal_diffusivity, time);

        ErrorMetrics result =
            compute_error_metrics(analytical_temperature_field, numerical_temperature_field);

        std::cout << std::setprecision(10) << "steps: " << steps << '\n';

        std::cout << std::setprecision(10) << "mae: " << result.mean_absolute_error << '\n';
        std::cout << std::setprecision(10) << "rmse: " << result.root_mean_square_error << '\n';
        std::cout << std::setprecision(10) << '\n';

        csv << ' ' << ',' << ' ' << ',' << ' ' << ',' << ' ' << ',' << steps << ','
            << result.mean_absolute_error << ',' << result.root_mean_square_error << '\n';
    }
}
}  // namespace error_experiment

int main() {
    auto data = common::make_run_timestamp();
    std::string experiment_name = "error_experiment";
    std::string filename = "unstable_dt_experiment";
    error_experiment::benchmark(data, experiment_name, filename,
                                error_experiment::unstable_dt_config);
}
