#include "test_heat_diffusion.h"

namespace {

const double thermal_diffusivity = 1e-5;
const int steps = 16;
const double dt = 0.01;
const std::size_t thread_count = 3;
const double tolerance = 1e-5;

}  // namespace

int main() {
    // heat_diffusion::test_numerical_solve_step(0.001, 0.0001, 1e-6);
    // heat_diffusion::test_numberical_solve(0.001, 1000, 0.0001, 1e-5);
    heat_diffusion::test_numberical_solve_parallel(thermal_diffusivity, steps, dt, thread_count,
                                                   tolerance);
}
