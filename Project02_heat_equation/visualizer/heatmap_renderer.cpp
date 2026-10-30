#include "heatmap_renderer.h"

#include <RGFW.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "heat_diffusion.h"
#include "pixel_buffer.h"
#include "state.h"

namespace {

static std::size_t map_buffer_to_grid(heat_diffusion::TemperatureField& field,
                                      visualizer::PixelBuffer& buffer, std::int32_t& mx,
                                      std::int32_t& my) {
    const heat_diffusion::Grid2D& grid = field.get_grid();
    const std::vector<double>& temperature = field.get_temperature();
    const std::size_t nx = grid.nx;
    const std::size_t ny = grid.ny;

    const std::size_t buffer_width = buffer.width();
    const std::size_t buffer_height = buffer.height();

    const std::size_t pixels_per_node = std::min(buffer_width / nx, buffer_height / ny);
    const double nodes_per_pixel = 1.0 / pixels_per_node;
    const std::size_t row_offset = (buffer_width - (nx - 1) * pixels_per_node - 1) / 2;
    const std::size_t row_end = row_offset + (nx * pixels_per_node);
    const std::size_t column_offset = (buffer_height - (ny - 1) * pixels_per_node - 1) / 2;
    const std::size_t column_end = column_offset + (ny * pixels_per_node);

    std::size_t x =
        std::min(std::max(static_cast<std::int64_t>(mx - row_offset), static_cast<std::int64_t>(0)),
                  static_cast<std::int64_t>(pixels_per_node * nx - 1));
    std::size_t y = std::min(
        std::max(static_cast<std::int64_t>(column_end - 1 - my), static_cast<std::int64_t>(0)),
        static_cast<std::int64_t>(pixels_per_node * ny - 1));

    return x / pixels_per_node + (y / pixels_per_node) * nx;
}

static double temperature_change_calu(const double brush_power, const double elapsed_seconds,
                                      const double thermal_diffusivity, const double k) {
    double heat = brush_power * elapsed_seconds;
    return heat * thermal_diffusivity / k;
}

}  // namespace

namespace visualizer {

double bilinear_interpolate(const double t00, const double t10, const double t01, const double t11,
                            const double tx, double ty) {
    return (1 - tx) * (1 - ty) * t00 + tx * (1 - ty) * t10 + (1 - tx) * ty * t01 + tx * ty * t11;
}

void map_grid_to_buffer(heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                        double color_scale, const Rgba8& color_pos, const Rgba8& color_neg) {
    const heat_diffusion::Grid2D& grid = field.get_grid();
    const std::vector<double>& temperature = field.get_temperature();
    const std::size_t nx = grid.nx;
    const std::size_t ny = grid.ny;

    const double scale = 1 / color_scale;

    const std::size_t buffer_width = buffer.width();
    const std::size_t buffer_height = buffer.height();

    const std::size_t pixels_per_node = std::min(buffer_width / nx, buffer_height / ny);

    if (pixels_per_node == 0) {
        throw std::invalid_argument("画布太小了");
    }

    const double nodes_per_pixel = 1.0 / pixels_per_node;

    const std::size_t row_offset = (buffer_width - (nx - 1) * pixels_per_node - 1) / 2;

    const std::size_t column_offset = (buffer_height - (ny - 1) * pixels_per_node - 1) / 2;
    const std::size_t column_end = column_offset + (ny * pixels_per_node);

    for (std::size_t j = 0; j < ny - 1; j++) {
        const std::size_t n_count = pixels_per_node + (j == ny - 2 ? 1 : 0);

        for (std::size_t n = 0; n < n_count; n++) {
            std::size_t y = column_end - 1 - (pixels_per_node * j + n);

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

                    const double ratio = std::min(1.0, std::abs(pixel_temperature) * scale);

                    auto scale_channel = [&ratio](std::uint8_t a,
                                                  std::uint8_t background) -> std::uint8_t {
                        return static_cast<std::uint8_t>(
                            std::lround(ratio * (a - background) + background));
                    };

                    std::size_t x = pixels_per_node * i + m + row_offset;

                    const Rgba8 backgroud = {245, 245, 245, 255};

                    Rgba8 pixel_color = backgroud;

                    if (pixel_temperature > 0) {
                        pixel_color = {scale_channel(color_pos.r, backgroud.r),
                                       scale_channel(color_pos.g, backgroud.g),
                                       scale_channel(color_pos.b, backgroud.b), 255};
                    } else {
                        pixel_color = {scale_channel(color_neg.r, backgroud.r),
                                       scale_channel(color_neg.g, backgroud.g),
                                       scale_channel(color_neg.b, backgroud.b), 255};
                    }

                    buffer.set_pixel(x, y, pixel_color);
                }
            }
        }
    }
}

void test_mouse_position(RGFW_window* window, RGFW_event& event, std::int32_t& mx,
                         std::int32_t& my) {
    if (RGFW_window_isMouseDown(window, RGFW_mouseLeft)) {
        RGFW_window_getMouse(window, &mx, &my);
        printf("You clicking at x: %d, y: %d\n", mx, my);
    }

    while (RGFW_window_checkEvent(window, &event)) {
        RGFW_window_getMouse(window, &mx, &my);

        if (event.type == RGFW_mouseButtonPressed && event.button.value == RGFW_mouseLeft) {
            printf("You clicked at x: %d, y: %d\n", mx, my);
        };
    }
}

void apply_heat_brush(RGFW_window* window, RGFW_event& event, std::int32_t& mx, std::int32_t& my,
                      heat_diffusion::TemperatureField& field, PixelBuffer& buffer,
                      const double brush_power, const double elapsed_seconds,
                      const double thermal_diffusivity, const double k) {
    if (RGFW_window_isMouseDown(window, RGFW_mouseLeft)) {
        RGFW_window_getMouse(window, &mx, &my);
        double dT = temperature_change_calu(brush_power, elapsed_seconds, thermal_diffusivity, k);
        std::size_t gird_index = map_buffer_to_grid(field, buffer, mx, my);
        std::vector<double>& temperature = field.get_temperature();
        printf("temperature: %f\n", temperature[gird_index]);
        temperature[gird_index] += dT;

        printf("temperature: %f\n", temperature[gird_index]);
    }
    if (RGFW_window_isMouseDown(window, RGFW_mouseRight)) {
        RGFW_window_getMouse(window, &mx, &my);
        double dT =
            -1.0 * temperature_change_calu(brush_power, elapsed_seconds, thermal_diffusivity, k);
        std::size_t gird_index = map_buffer_to_grid(field, buffer, mx, my);
        std::vector<double>& temperature = field.get_temperature();
        printf("temperature: %f\n", temperature[gird_index]);
        temperature[gird_index] += dT;

        printf("temperature: %f\n", temperature[gird_index]);
    }
}

}  // namespace visualizer
