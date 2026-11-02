#include "state.h"
namespace heat_diffusion {

void launch_heat_diffusion_step(heat_diffusion::Grid2D& grid, NuemricalWorkspace& workspace,
                                double*& old_temperature, double*& new_temperature,
                                const int threads_per_block, const int step);
}
