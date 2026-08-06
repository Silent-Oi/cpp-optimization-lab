#define RGFW_IMPLEMENTATION
#include <RGFW.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "oscillator_batch.h"
#include "pixel_buffer.h"
#include "state.h"

namespace fs = std::filesystem;

namespace visualizer {

const Rgba8 blue{0, 180, 255, 255};     // 青蓝
const Rgba8 light{220, 240, 255, 255};  // 亮白蓝
const Rgba8 gold{255, 220, 80, 255};    // 金黄
const Rgba8 white{255, 255, 255, 255};

const Rgba8 light_blue{100, 190, 255, 255};
const Rgba8 mid_white{245, 245, 245, 255};  // 暖白：中阻尼
const Rgba8 light_red{255, 125, 135, 255};  // 浅红：高阻尼

static void output_ppm(const PixelBuffer& buffer, const fs::path& output_path) {
    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        throw std::runtime_error("结果生成失败");
    }

    output << "P6\n"
           << buffer.width() << ' ' << buffer.height() << "\n"
           << "255\n";

    const std::vector<std::uint8_t>& rgba = buffer.bytes();

    const std::size_t pixel_number =
        static_cast<std::size_t>(buffer.width()) * static_cast<std::size_t>(buffer.height());

    for (std::size_t pixel = 0; pixel < pixel_number; ++pixel) {
        output.put(static_cast<char>(rgba[4 * pixel]));
        output.put(static_cast<char>(rgba[4 * pixel + 1]));
        output.put(static_cast<char>(rgba[4 * pixel + 2]));
    }
}
static Coordinate mapping_phase_to_buffer(const double position, const double velocity,
                                          const PixelBuffer& buffer) {
    int center_x = buffer.width() * 0.5;
    int center_y = buffer.height() * 0.5;

    int coord_x = static_cast<int>(std::lround(position * center_x + center_x));
    int coord_y = static_cast<int>(std::lround(-1 * velocity * center_y + center_y));

    if (coord_x >= buffer.width() || coord_x < 0 || coord_y >= buffer.height() || coord_y < 0) {
        return {-1, -1};
    }
    return {coord_x, coord_y};
}

static void norm_velocity(oscillator::OscillatorSoABatch& oscillators) {
    const std::size_t numbers = oscillators.omega.size();
    for (std::size_t i = 0; i < numbers; ++i) {
        double norm_velocity = oscillators.velocity[i] * oscillators.omega[i];
        oscillators.velocity[i] = norm_velocity;
    }
}

static Rgba8 lerp_color(double ratio, const Rgba8& color_a, const Rgba8& color_b) {
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

static std::vector<Rgba8> color_oscillator(const oscillator::OscillatorSoABatch& oscillators,
                                           PixelBuffer& buffer, const Rgba8& color_low,
                                           const Rgba8& color_mid, const Rgba8& color_high) {
    const std::size_t numbers = oscillators.omega.size();
    std::vector<Rgba8> colors(numbers);
    for (std::size_t i = 0; i < numbers; ++i) {
        Coordinate coord = mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
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

static std::vector<Coordinate> coord_oscillator(const oscillator::OscillatorSoABatch& oscillators,
                                                PixelBuffer& buffer) {
    const std::size_t oscillator_number = oscillators.omega.size();
    std::vector<Coordinate> oscillator_coords(oscillator_number);

    for (std::size_t i = 0; i < oscillator_number; ++i) {
        oscillator_coords[i] = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
    }
    return oscillator_coords;
}

static void draw_mix_color(const oscillator::OscillatorSoABatch& oscillators, PixelBuffer& buffer,
                           const std::vector<Rgba8>& colors,
                           const std::vector<Coordinate>& coords) {
    const std::size_t pixel_number =
        static_cast<std::size_t>(buffer.width()) * static_cast<std::size_t>(buffer.height());
    const std::size_t oscillator_number = oscillators.omega.size();
    std::vector<PixelAccumulator> accumulators(pixel_number);

    std::vector<Rgba8> mixed_color(oscillator_number);

    for (std::size_t i = 0; i < oscillator_number; ++i) {
        const int x = coords[i].x;
        const int y = coords[i].y;
        if (x < 0 || y < 0 || x >= buffer.width() || y >= buffer.height()) {
            continue;
        }
        std::size_t pixel = static_cast<std::size_t>(buffer.width() * y + x);
        accumulators[pixel].sum_r += colors[i].r;
        accumulators[pixel].sum_g += colors[i].g;
        accumulators[pixel].sum_b += colors[i].b;
        ++accumulators[pixel].count;
    }

    constexpr double reference_count = 3.0;

    for (std::size_t i = 0; i < oscillator_number; ++i) {
        const int x = coords[i].x;
        const int y = coords[i].y;
        if (x < 0 || y < 0 || x >= buffer.width() || y >= buffer.height()) {
            continue;
        }
        std::size_t pixel = static_cast<std::size_t>(buffer.width() * y + x);

        uint32_t count = accumulators[pixel].count;
        if (count == 0) {
            continue;
        }

        const double intensity = std::clamp(
            std::log1p(static_cast<double>(count)) / std::log1p(reference_count), 0.0, 1.0);

        mixed_color[i].r = static_cast<std::uint8_t>(accumulators[pixel].sum_r / count * intensity);
        mixed_color[i].g = static_cast<std::uint8_t>(accumulators[pixel].sum_g / count * intensity);
        mixed_color[i].b = static_cast<std::uint8_t>(accumulators[pixel].sum_b / count * intensity);
        mixed_color[i].a = 255;
        buffer.set_pixel(x, y, mixed_color[i]);
    }
}

static void draw_oscillator(const oscillator::OscillatorSoABatch& oscillators, PixelBuffer& buffer,
                            const std::vector<Rgba8>& colors,
                            const std::vector<Coordinate>& coords) {
    const std::size_t numbers = oscillators.omega.size();
    for (std::size_t i = 0; i < numbers; ++i) {
        buffer.set_pixel(coords[i].x, coords[i].y, colors[i]);
    }
}

}  // namespace visualizer

int main() {
    constexpr double dt = 0.001;
    constexpr int seed = 1234;
    constexpr int width = 1025;
    constexpr int height = 1025;

    const fs::path output_directory = fs::path(PROJECT01_SOURCE_DIR) / "results";
    fs::create_directories(output_directory);
    fs::path output_path = output_directory / "oscillators_location.ppm";

    visualizer::PixelBuffer buffer(width, height);
    oscillator::OscillatorSoABatch initial_oscillators =
        oscillator::make_oscillator_soa_batch(30000, dt, seed);
    oscillator::OscillatorSoABatch work_oscillators = initial_oscillators;

    const std::size_t numbers = work_oscillators.omega.size();
    visualizer::norm_velocity(work_oscillators);
    std::vector<visualizer::Rgba8> colors =
        visualizer::color_oscillator(work_oscillators, buffer, visualizer::light_blue,
                                     visualizer::mid_white, visualizer::light_red);

    RGFW_window* window = RGFW_createWindow("Oscillator Visualizer", 0, 0, width, height,
                                            RGFW_windowCenter | RGFW_windowNoResize);
    if (window == nullptr) {
        RGFW_deinit();
        return 1;
    }
    RGFW_window_setExitKey(window, RGFW_escape);
    RGFW_surface* surface = RGFW_createSurface(buffer.data(), width, height, RGFW_formatRGBA8);
    if (surface == nullptr) {
        RGFW_window_close(window);
        RGFW_deinit();
        return 1;
    }

    bool paused = false;
    auto previous_time = std::chrono::steady_clock::now();
    double simulation_accumulator = 0.0;
    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_pollEvents();
        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>(' '))) {
            paused = !paused;
        }

        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>('r'))) {
            work_oscillators = initial_oscillators;
            visualizer::norm_velocity(work_oscillators);
            paused = true;
            buffer.clear();
            std::vector<visualizer::Coordinate> coords =
                visualizer::coord_oscillator(work_oscillators, buffer);
            buffer.clear();
            visualizer::draw_mix_color(work_oscillators, buffer, colors, coords);

            RGFW_window_blitSurface(window, surface);
        }

        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>('n'))) {
            paused = true;
            oscillator::update_soa_batch_step(work_oscillators);
            buffer.clear();
            std::vector<visualizer::Coordinate> coords =
                visualizer::coord_oscillator(work_oscillators, buffer);
            buffer.clear();
            visualizer::draw_mix_color(work_oscillators, buffer, colors, coords);

            RGFW_window_blitSurface(window, surface);
        }

        const auto current_time = std::chrono::steady_clock::now();
        const double elapsed_seconds =
            std::chrono::duration<double>(current_time - previous_time).count();
        simulation_accumulator += 0.1 * elapsed_seconds;
        previous_time = current_time;
        int steps = simulation_accumulator / dt;
        simulation_accumulator -= steps * dt;

        if (!paused) {
            oscillator::update_soa_batch(work_oscillators, steps);
            std::vector<visualizer::Coordinate> coords =
                visualizer::coord_oscillator(work_oscillators, buffer);
            buffer.clear();
            visualizer::draw_mix_color(work_oscillators, buffer, colors, coords);
            RGFW_window_blitSurface(window, surface);
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    }
    RGFW_surface_free(surface);
    RGFW_window_close(window);
    RGFW_deinit();
}
