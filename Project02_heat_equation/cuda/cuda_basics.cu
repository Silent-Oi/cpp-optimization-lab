#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <device_launch_parameters.h>
#include <crt/host_defines.h>
#include <cstdio>
namespace {
__global__ void double_values(float* values, int n) {
    int i = threadIdx.x;
    if (i < n) {
        values[i] *= 2;
    }
}
}  // namespace
int main() {
    float values[4] = {1.0f, 2.0f, 3.0f, 4.0f};

    float* device_values = nullptr;

    cudaError_t status = cudaMalloc(&device_values, sizeof(values));

    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        return 1;
    }

    status = cudaMemcpy(device_values, values, sizeof(values), cudaMemcpyHostToDevice);

    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(device_values);
        return 1;
    }

    double_values<<<1, 4>>>(device_values, 4);

    status = cudaGetLastError();
    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(device_values);
        return 1;
    }

    status = cudaDeviceSynchronize();

    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(device_values);
        return 1;
    }

    status = cudaMemcpy(values, device_values, sizeof(values), cudaMemcpyDeviceToHost);

    if (status != cudaSuccess) {
        printf("%s\n", cudaGetErrorString(status));
        cudaFree(device_values);
        return 1;
    }

    cudaFree(device_values);

    for (auto i : values) {
        printf("%f\n", i);
    }

}
