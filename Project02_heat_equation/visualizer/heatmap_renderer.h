#include "heat_diffusion.h"
#include "pixel_buffer.h"
#include "state.h"
namespace visualizer {

void map_grid_to_buffer(heat_diffusion::TemperatureField field, PixelBuffer& buffer,
                        const double color_scale, const Rgba8& color_max, const Rgba8& color_min);
}  // namespace visualizer
