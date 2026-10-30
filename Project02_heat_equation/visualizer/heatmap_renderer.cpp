#include "heatmap_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "heat_diffusion.h"
#include "pixel_buffer.h"
#include "state.h"

namespace visualizer {

double bilinear_interpolate(const double t00, const double t10, const double t01, const double t11,
                            const double tx, double ty) {
    return (1 - tx) * (1 - ty) * t00 + tx * (1 - ty) * t10 + (1 - tx) * ty * t01 + tx * ty * t11;
}

void map_grid_to_buffer(heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        const double color_scale, const Rgba8& color_pos, const Rgba8& color_neg) {
    const heat_diffusion::Grid2D& grid = field.get_grid();
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
        const std::size_t n_count = pixels_per_node + (j == ny - 2 ? 1 : 0);
        for (std::size_t n = 0; n < n_count; n++) {
            const double ty = n * nodes_per_pixel;
            for (std::size_t i = 0; i < nx - 1; i++) {
                const std::size_t m_count = pixels_per_node + (i == nx - 2 ? 1 : 0);
                std::size_t gird_index1 = j * nx + i;
                std::size_t gird_index3 = gird_index1 + nx;
                std::size_t gird_index2 = gird_index1 + 1;
                std::size_t gird_index4 = gird_index3 + 1;

                const double temperature1 = temperature[gird_index1];
                const double temperature2 = temperature[gird_index2];
                const double temperature3 = temperature[gird_index3];
                const double temperature4 = temperature[gird_index4];

                for (std::size_t m = 0; m < m_count; m++) {
                    const double tx = m * nodes_per_pixel;

                    const double pixel_temperature = bilinear_interpolate(
                        temperature1, temperature2, temperature3, temperature4, tx, ty);

                    const double ratio = std::min(1.0, std::abs(pixel_temperature) / color_scale);

                    auto scale_channel = [&ratio](std::uint8_t a) -> std::uint8_t {
                        return static_cast<std::uint8_t>(std::lround(ratio * a));
                    };

                    std::size_t x = pixels_per_node * i + m + row_offset;
                    std::size_t y = column_end - 1 - (pixels_per_node * j + n);

                    Rgba8 pixel_color = {255, 255, 255, 255};

                    if (pixel_temperature > 0) {
                        pixel_color = {scale_channel(color_pos.r), scale_channel(color_pos.g),
                                       scale_channel(color_pos.b), 255};
                    } else {
                        pixel_color = {scale_channel(color_neg.r), scale_channel(color_neg.g),
                                       scale_channel(color_neg.b), 255};
                    }

                    buffer.set_pixel(x, y, pixel_color);
                }
            }
        }
    }
}

}  // namespace visualizer
