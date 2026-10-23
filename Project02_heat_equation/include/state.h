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

struct NuemricalWorkspace {
    double rx;
    double ry;
    double rc;
    std::vector<double> temperature_cache;
};

}  // namespace heat_diffusion
