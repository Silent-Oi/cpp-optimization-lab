#include "phase_space_renderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "pixel_buffer.h"
#include "state.h"
namespace visualizer {

Coordinate mapping_phase_to_buffer(const double position, const double velocity,
                                   const PixelBuffer& buffer) {
    // 将 [-1, 1] 内的相空间坐标映射到像素坐标。
    int center_x = buffer.width() * 0.5;
    int center_y = buffer.height() * 0.5;

    int coord_x = static_cast<int>(std::lround(position * center_x + center_x));
    int coord_y = static_cast<int>(std::lround(-1 * velocity * center_y + center_y));

    if (coord_x >= buffer.width() || coord_x < 0 || coord_y >= buffer.height() || coord_y < 0) {
        return {-1, -1};
    }
    return {coord_x, coord_y};
}

void convert_phase_velocity_to_physical(oscillator::OscillatorSoABatch& oscillators) {
    const std::size_t numbers = oscillators.omega.size();
    for (std::size_t i = 0; i < numbers; ++i) {
        double convert_phase_velocity_to_physical = oscillators.velocity[i] * oscillators.omega[i];
        oscillators.velocity[i] = convert_phase_velocity_to_physical;
    }
}

Rgba8 lerp_color(double ratio, const Rgba8& color_a, const Rgba8& color_b) {
    auto lerp_channel = [ratio](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
        return static_cast<std::uint8_t>(std::lround(ratio * (b - a) + a));
    };

    return {
        lerp_channel(color_a.r, color_b.r),
        lerp_channel(color_a.g, color_b.g),
        lerp_channel(color_a.b, color_b.b),
        lerp_channel(color_a.a, color_b.a),
    };
}

std::vector<Rgba8> make_oscillator_colors(const oscillator::OscillatorSoABatch& oscillators,
                                          PixelBuffer& buffer, const Rgba8& color_low,
                                          const Rgba8& color_mid, const Rgba8& color_high) {
    const std::size_t numbers = oscillators.omega.size();
    std::vector<Rgba8> colors(numbers);
    for (std::size_t i = 0; i < numbers; ++i) {
        double zeta = oscillators.zeta[i];
        if (zeta < 0.45) {
            Rgba8 color = lerp_color(zeta / 0.45, color_low, color_mid);
            colors[i] = color;
        }

        else {
            Rgba8 color = lerp_color((zeta - 0.45) / 0.45, color_mid, color_high);
            colors[i] = color;
        }
    }
    return colors;
}

PhaseSpaceRenderer::PhaseSpaceRenderer(const oscillator::OscillatorSoABatch& oscillators,
                                       PixelBuffer& buffer, const Rgba8& low_color,
                                       const Rgba8& mid_color, const Rgba8& high_color)
    : accumulators_(buffer.pixel_count()) {
    oscillator_colors_ =
        make_oscillator_colors(oscillators, buffer, low_color, mid_color, high_color);
}

void PhaseSpaceRenderer::render(const oscillator::OscillatorSoABatch& oscillators,
                                PixelBuffer& buffer, const double reference_count) {
    // 只清除上一帧命中的像素和累加结果。
    for (std::size_t pixel : touched_pixels_) {
        buffer.set_pixel(pixel, black);
        accumulators_[pixel] = {};
    }

    touched_pixels_.clear();

    const std::size_t oscillator_number = oscillators.omega.size();
    // 先按像素累加颜色和振子数量。
    for (std::size_t i = 0; i < oscillator_number; ++i) {
        Coordinate oscillator_coords = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
        const int x = oscillator_coords.x;
        const int y = oscillator_coords.y;
        if (x < 0 || y < 0 || x >= buffer.width() || y >= buffer.height()) {
            continue;
        }
        std::size_t pixel = static_cast<std::size_t>(buffer.width() * y + x);
        accumulators_[pixel].sum_r += oscillator_colors_[i].r;
        accumulators_[pixel].sum_g += oscillator_colors_[i].g;
        accumulators_[pixel].sum_b += oscillator_colors_[i].b;
        ++accumulators_[pixel].count;
        if (accumulators_[pixel].count == 1) {
            touched_pixels_.push_back(pixel);
        }
    }

    // 每个命中像素只混色并写入一次。
    for (std::size_t pixel : touched_pixels_) {
        if (pixel >= buffer.pixel_count()) {
            continue;
        }

        uint32_t count = accumulators_[pixel].count;
        const double intensity = std::clamp(
            std::log1p(static_cast<double>(count)) / std::log1p(reference_count), 0.0, 1.0);

        const auto channel = [&](std::uint32_t sum) -> std::uint8_t {
            return static_cast<std::uint8_t>((static_cast<double>(sum) / count) * intensity);
        };

        Rgba8 color = {channel(accumulators_[pixel].sum_r), channel(accumulators_[pixel].sum_g),
                       channel(accumulators_[pixel].sum_b), 255};
        buffer.set_pixel(pixel, color);
    }
}

}  // namespace visualizer
