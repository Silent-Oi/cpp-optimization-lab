#include "test_heat_diffusion.h"

int main() {
    heat_diffusion::test_numerical_solve_step(0.0001);
    heat_diffusion::test_numberical_solve(10, 0.0001);
}
