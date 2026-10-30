#pragma once
#include <RGFW.h>
#include "heat_diffusion.h"
#include "pixel_buffer.h"
#include <cstdint>

namespace visualizer {

double bilinear_interpolate(const double t00, const double t10, const double t01, const double t11,
                            const double tx, double ty);

void map_grid_to_buffer(heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        const double color_scale, const Rgba8& color_pos, const Rgba8& color_neg);

void test_mouse_position(RGFW_window* window, RGFW_event& event, std::int32_t& mx,
                         std::int32_t& my);

void apply_heat_brush(RGFW_window* window, RGFW_event& event, std::int32_t& mx, std::int32_t& my,
                        heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        const double brush_power, const double elapsed_seconds,
                        const double thermal_diffusivity, const double k);
}  // namespace visualizer
