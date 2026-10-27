#include "heatmap_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

#include "heat_diffusion.h"
#include "pixel_buffer.h"
#include "state.h"

namespace visualizer {

void map_grid_to_buffer(heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        const double color_scale, const Rgba8& color_max, const Rgba8& color_min) {
    const heat_diffusion::Grid2D grid = field.get_grid();
    const std::vector<double>& temperature = field.get_temperature();
    const std::size_t nx = grid.nx;
    const std::size_t ny = grid.ny;

    const std::size_t buffer_width = buffer.width();
    const std::size_t buffer_height = buffer.height();

    const std::size_t pixels_per_node = std::min(buffer_width / nx, buffer_height / ny);

    if (pixels_per_node == 0) {
        throw std::invalid_argument("画布太小了");
    }

    const double nodes_per_pixel = 1.0 / pixels_per_node;

    const std::size_t row_offset = (buffer_width - nx * pixels_per_node) / 2;
    const std::size_t row_end = row_offset + (nx * pixels_per_node);

    const std::size_t column_offset = (buffer_height - ny * pixels_per_node) / 2;
    const std::size_t column_end = column_offset + (ny * pixels_per_node);

    for (std::size_t j = 0; j < ny - 1; j++) {
        for (std::size_t n = 0; n <= pixels_per_node; n++) {
            for (std::size_t i = 0; i < nx - 1; i++) {
                for (std::size_t m = 0; m <= pixels_per_node; m++) {
                    std::size_t gird_index1 = j * nx + i;
                    std::size_t gird_index3 = gird_index1 + nx;
                    std::size_t gird_index2 = gird_index1 + 1;
                    std::size_t gird_index4 = gird_index3 + 1;

                    double dist1 = std::sqrt((m * nodes_per_pixel) * (m * nodes_per_pixel) +
                                             (n * nodes_per_pixel) * (n * nodes_per_pixel));
                    double dist2 = std::sqrt((1 - m * nodes_per_pixel) * (1 - m * nodes_per_pixel) +
                                             (n * nodes_per_pixel) * (n * nodes_per_pixel));
                    double dist3 = std::sqrt((m * nodes_per_pixel) * (m * nodes_per_pixel) +
                                             (1 - n * nodes_per_pixel) * (1 - n * nodes_per_pixel));
                    double dist4 = std::sqrt((1 - m * nodes_per_pixel) * (1 - m * nodes_per_pixel) +
                                             (1 - n * nodes_per_pixel) * (1 - n * nodes_per_pixel));

                    auto lerp_channel = [&color_scale](const double temperature, const double dist,
                                                       std::uint8_t channel_pos,
                                                       std::uint8_t channel_neg) {
                        if (temperature >= 0) {
                            return std::min(temperature / color_scale, 1.0) *
                                   std::max((1.0 - dist), 0.0) * channel_pos;
                        } else {
                            return std::min(-1.0 * temperature / color_scale, 1.0) *
                                   std::max((1.0 - dist), 0.0) * channel_neg;
                        }
                    };

                    std::uint8_t a = 255;
                    std::uint8_t r =
                        (lerp_channel(temperature[gird_index1], dist1, color_max.r, color_min.r) +
                         lerp_channel(temperature[gird_index2], dist2, color_max.r, color_min.r) +
                         lerp_channel(temperature[gird_index3], dist3, color_max.r, color_min.r) +
                         lerp_channel(temperature[gird_index4], dist4, color_max.r, color_min.r)) /
                        4;
                    std::uint8_t g =
                        (lerp_channel(temperature[gird_index1], dist1, color_max.g, color_min.g) +
                         lerp_channel(temperature[gird_index2], dist2, color_max.g, color_min.g) +
                         lerp_channel(temperature[gird_index3], dist3, color_max.g, color_min.g) +
                         lerp_channel(temperature[gird_index4], dist4, color_max.g, color_min.g)) /
                        4;
                    std::uint8_t b =
                        (lerp_channel(temperature[gird_index1], dist1, color_max.b, color_min.b) +
                         lerp_channel(temperature[gird_index2], dist2, color_max.b, color_min.b) +
                         lerp_channel(temperature[gird_index3], dist3, color_max.b, color_min.b) +
                         lerp_channel(temperature[gird_index4], dist4, color_max.b, color_min.b)) /
                        4;
                    Rgba8 lerp_color{r, g, b, a};

                    std::size_t x = pixels_per_node * i + m + row_offset;
                    std::size_t y = column_end - 1 - (pixels_per_node * j + n);

                    buffer.set_pixel(x, y, lerp_color);
                }
            }
        }
    }
}

}  // namespace visualizer
