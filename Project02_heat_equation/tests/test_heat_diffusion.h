#pragma once

#include <cmath>
namespace heat_diffusion {

void test_numerical_solve_step(const double thermal_diffusivity, const double dt, double tolerance);
void test_numberical_solve(const double thermal_diffusivity, int steps, double dt,
                           double tolerance);

void test_numberical_solve_parallel(const double thermal_diffusivity, const int steps,
                                    const double dt, const std::size_t thread_cout,
                                    const double tolerance);

}  // namespace heat_diffusion
