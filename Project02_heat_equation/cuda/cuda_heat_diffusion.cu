#include <cuda_heat_diffusion.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <driver_types.h>
#include <crt/host_defines.h>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "heat_diffusion.h"
#include "state.h"

namespace {

__global__ void heat_diffusion_step_kernel(double* old_temperature, double* new_temperature,
                                           const int number_x, const int number_y, const double rx,
                                           const double ry, const double rc) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int i = index % number_x;
    int j = (index - i) / number_x;

    if (i > 0 && i < number_x - 1 && j > 0 && j < number_y - 1) {
        double orign_temperature = old_temperature[index];
        double left_temperature = old_temperature[index - 1];
        double right_temperature = old_temperature[index + 1];
        double down_temperature = old_temperature[index - number_x];
        double up_temperature = old_temperature[index + number_x];

        new_temperature[index] = rc * orign_temperature +
                                 rx * (right_temperature + left_temperature) +
                                 ry * (up_temperature + down_temperature);
    }
}

}  // namespace

namespace heat_diffusion {

void launch_heat_diffusion_step(TemperatureField& field, NuemricalWorkspace& workspace,
                                const int block_count, const int threads_per_block) {
    const heat_diffusion::Grid2D& grid = field.get_grid();
    std::vector<double> temperature = field.get_temperature();

    const int number_y = grid.ny;
    const int number_x = grid.nx;
    const double rx = workspace.rx;
    const double ry = workspace.ry;
    const double rc = workspace.rc;

    std::size_t temperature_bytes = temperature.size() * sizeof(double);
    double* old_temperature = nullptr;
    double* new_temperature = nullptr;

    cudaError_t status = cudaMalloc(&old_temperature, temperature_bytes);
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        throw std::invalid_argument("报错了");
    }

    status = cudaMalloc(&new_temperature, temperature_bytes);
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        throw std::invalid_argument("报错了");
    }

    status =
        cudaMemcpy(old_temperature, temperature.data(), temperature_bytes, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        throw std::invalid_argument("报错了");
    }

    status = cudaMemcpy(new_temperature, workspace.temperature_cache.data(), temperature_bytes,
                        cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        throw std::invalid_argument("报错了");
    }

    heat_diffusion_step_kernel<<<block_count, threads_per_block>>>(old_temperature, new_temperature,
                                                                   number_x, number_y, rx, ry, rc);
    status = cudaGetLastError();
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(old_temperature);
        cudaFree(new_temperature);
        throw std::invalid_argument("报错了");
    }

    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(old_temperature);
        cudaFree(new_temperature);
        throw std::invalid_argument("报错了");
    }

    status = cudaMemcpy(workspace.temperature_cache.data(), new_temperature, temperature_bytes,
                        cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(old_temperature);
        cudaFree(new_temperature);
        throw std::invalid_argument("报错了");
    }

    cudaFree(old_temperature);
    cudaFree(new_temperature);
}

}  // namespace heat_diffusion
