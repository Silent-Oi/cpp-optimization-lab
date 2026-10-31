#include "heat_diffusion.h"
#include "state.h"
namespace heat_diffusion {

void launch_heat_diffusion_step(TemperatureField& field, NuemricalWorkspace& workspace,
                                const int block_count, const int threads_per_block);
}
