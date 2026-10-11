#include <vector>

#include "state.h"

namespace heat_diffusion {

TemperatureGrid make_temperature_grid(const Grid2D& grid, InitialField field,
                                      double initial_temperature);

namespace analytical {
void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double time);
}

}  // namespace heat_diffusion
