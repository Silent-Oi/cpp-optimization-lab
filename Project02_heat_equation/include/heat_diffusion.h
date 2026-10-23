#pragma once
#include <vector>

#include "state.h"

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

TemperatureGrid make_temperature_grid(const Grid2D& grid, InitialField initial_field_type,
                                      double initial_temperature, double thermal_diffusivity,
                                      double dt);

class TemperatureField {
   public:
    TemperatureField(const Grid2D grid, InitialField initial_field_type,
                     double initial_temperature);
    const NuemricalWorkspace make_numerical_workspace(double thermal_diffusivity,
                                       double dt) const;
    const Grid2D& get_grid() const;
    const std::vector<double>& get_temperature() const;

    const void numerical_solve_step(NuemricalWorkspace& workspace);
    const void numerical_solve(NuemricalWorkspace& workspace, const int steps);
    const void analytical_solve(const double thermal_diffusivity, const double time);

   private:
    Grid2D grid_;
    std::vector<double> temperature_;
};

// ============================================================================
// 中心差分解
// ============================================================================

namespace numerical {

void solve_step(TemperatureGrid& temperature_field);
void solve(TemperatureGrid& temperature_field, int steps);

}  // namespace numerical

// ============================================================================
// 正弦场解析解
// ============================================================================

namespace analytical {

void solve(TemperatureGrid& temperature_field, double thermal_diffusivity, double time);

}

}  // namespace heat_diffusion
