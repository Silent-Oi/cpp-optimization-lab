#pragma once
#include <barrier>
#include <vector>

#include "state.h"

namespace heat_diffusion {

// ---- 创建温度场 --------------------------------------------------------------

class TemperatureField {
   public:
    TemperatureField(const Grid2D grid, InitialField initial_field_type,
                     double initial_temperature);
    NuemricalWorkspace make_numerical_workspace(double thermal_diffusivity, double dt) const;
    const Grid2D& get_grid() const;
    std::vector<double>& get_temperature();

    void numerical_solve_step(NuemricalWorkspace& workspace);
    void numerical_solve_step(NuemricalWorkspace& workspace, const std::size_t begin,
                              const std::size_t end);

    void numerical_solve(NuemricalWorkspace& workspace, const int steps);
    void analytical_solve(const double thermal_diffusivity, const double time);

    void numerical_solve_parallel(NuemricalWorkspace& workspace, const int steps,
                                  const std::size_t thread_count);

   private:
    Grid2D grid_;
    std::vector<double> temperature_;
};

}  // namespace heat_diffusion
