#pragma once
#include <cmath>
#include <vector>

namespace heat_diffusion {

struct Grid2D {
    std::size_t nx;
    std::size_t ny;
    double dx;
    double dy;
};

enum class InitialField { Uniform, SineMode };

struct TemperatureGrid {
    Grid2D grid;
    double rx;
    double ry;
    double rc;
    // index = y*nx + x; [0, nx*ny)
    std::vector<double> temperature;
    std::vector<double> temperature_cache;
};

struct NuemricalWorkspace {
    double rx;
    double ry;
    double rc;
    std::vector<double> temperature_cache;
};

}  // namespace heat_diffusion
