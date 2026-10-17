#include "test_heat_diffusion.h"

int main() {
    heat_diffusion::test_numerical_solve_step(0.001, 0.0001, 1e-6);
    heat_diffusion::test_numberical_solve(0.001, 1000, 0.0001, 1e-5);
}
