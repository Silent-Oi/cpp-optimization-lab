#include <chrono>
#include <iomanip>
#include <iostream>

#include "heat_diffusion.h"
#include "state.h"

namespace {

heat_diffusion::Grid2D default_grid{
    1024,
    1024,
    1.0 / 1021,
    1.0 / 1021,
};
heat_diffusion::InitialField initial_field_type = heat_diffusion::InitialField::SineMode;

double initial_temperature = 64;
double thermal_diffusivity = 1e-5;
int steps = 512;
double dt = 0.01;
int threads_per_block = 128;
std::size_t cycle_count = 8;

struct CudaTiming {
    double total_time_ns;
    float kernel_time_ns;
};

CudaTiming benchmark(heat_diffusion::TemperatureField& bench_field,
                     heat_diffusion::NuemricalWorkspace& workspace, const int steps,
                     const int threads_per_block) {
    const auto start = std::chrono::steady_clock::now();
    float kernel_time_ns = bench_field.numerical_solve_cuda(workspace, steps, threads_per_block);
    const auto end = std::chrono::steady_clock::now();
    double total_time_ns = std::chrono::duration<double>(end - start).count() * 1e9;
    return {
        total_time_ns,
        kernel_time_ns,
    };
}


}  // namespace

int main() {
    std::cout << std::setprecision(10);

    heat_diffusion::TemperatureField initial_field(default_grid, initial_field_type,
                                                   initial_temperature);

    heat_diffusion::NuemricalWorkspace workspace =
        initial_field.make_numerical_workspace(thermal_diffusivity, dt);

    heat_diffusion::TemperatureField work_filed = initial_field;
    std::size_t calu_counts = default_grid.nx * default_grid.ny * steps;

    double total_ns_per_calu = 0.0;
    float kernel_ns_per_calu = 0.0;

    for (std::size_t i = 0; i < cycle_count; ++i) {
        work_filed = initial_field;
        benchmark(work_filed, workspace, steps, threads_per_block);
        work_filed = initial_field;
        work_filed = initial_field;
        CudaTiming cuda_time = benchmark(work_filed, workspace, steps, threads_per_block);
        
        total_ns_per_calu += cuda_time.total_time_ns / calu_counts;
        kernel_ns_per_calu += cuda_time.kernel_time_ns / calu_counts;
    }

    total_ns_per_calu = total_ns_per_calu / cycle_count;
    kernel_ns_per_calu = kernel_ns_per_calu / cycle_count;

    std::cout << "total_ns_per_calu: " << total_ns_per_calu << '\n';
    std::cout << "kernel_ns_per_calu: " << kernel_ns_per_calu << '\n';
    std::cout << '\n';
}
