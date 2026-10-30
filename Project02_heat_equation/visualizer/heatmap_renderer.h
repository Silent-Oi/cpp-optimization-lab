#pragma once

#include "heat_diffusion.h"
#include "pixel_buffer.h"

namespace visualizer {

double bilinear_interpolate(const double t00, const double t10, const double t01, const double t11,
                            const double tx, double ty);

void map_grid_to_buffer(heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        const double color_scale, const Rgba8& color_pos, const Rgba8& color_neg);
}  // namespace visualizer
