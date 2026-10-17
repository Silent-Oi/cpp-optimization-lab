#include <vector>

#include "state.h"

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

TemperatureGrid make_temperature_grid(const Grid2D& grid, InitialField field,
                                      double initial_temperature);

// ============================================================================
// 中心差分解
// ============================================================================

namespace numerical {

void solve_step(TemperatureGrid& temperature_field, double thermal_diffusivity, double dt);
void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double steps, double dt);

}  // namespace numerical

// ============================================================================
// 正弦场解析解
// ============================================================================

namespace analytical {

void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double time);

}

}  // namespace heat_diffusion
