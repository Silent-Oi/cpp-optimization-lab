#pragma once

namespace heat_diffusion {

void test_numerical_solve_step(const double thermal_diffusivity, const double dt, double tolerance);
void test_numberical_solve(const double thermal_diffusivity, int steps, double dt,
                           double tolerance);

}  // namespace heat_diffusion
