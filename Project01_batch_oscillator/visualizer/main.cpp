#define RGFW_IMPLEMENTATION
#include <RGFW.h>

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
static Coordnate mapping_phase_to_buffer(const double position, const double velocity,
                                         const PixelBuffer& buffer) {
    int center_x = buffer.width() * 0.5;
    int center_y = buffer.height() * 0.5;

    int cord_x = static_cast<int>(std::lround(position * center_x + center_x));
    int cord_y = static_cast<int>(std::lround(-1 * velocity * center_y + center_y));

    if (cord_x >= buffer.width() || cord_x < 0 || cord_y >= buffer.height() || cord_y < 0) {
        return {-1, -1};
    }
    return {cord_x, cord_y};
}

static void norm_velocity(oscillator::OscillatorSoABatch& oscillators) {
    const std::size_t numbers = oscillators.omega.size();
    for (std::size_t i = 0; i < numbers; ++i) {
        double norm_velocity = oscillators.velocity[i] * oscillators.omega[i];
        oscillators.velocity[i] = norm_velocity;
    }
}

}  // namespace visualizer

int main() {
    const fs::path output_directory = fs::path(PROJECT01_SOURCE_DIR) / "results";
    fs::create_directories(output_directory);
    fs::path output_path = output_directory / "oscillators_location.ppm";

    visualizer::Rgba8 white{255, 255, 255, 255};
    constexpr int width = 1025;
    constexpr int height = 1025;
     
    visualizer::PixelBuffer buffer(width, height);

    oscillator::OscillatorSoABatch oscillators =
        oscillator::make_oscillator_soa_batch(50000, 0.01, 1234);
    
    visualizer::norm_velocity(oscillators);
    const std::size_t numbers = oscillators.omega.size();

    for (std::size_t i = 0; i < numbers; ++i) {
        visualizer::Coordnate cord = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
        buffer.set_pixel(cord.x, cord.y, white);
    }

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

    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_pollEvents();
        RGFW_window_blitSurface(window, surface);
        oscillator::update_soa_batch_step(oscillators);
        buffer.clear();
        for (std::size_t i = 0; i < numbers; ++i) {
            visualizer::Coordnate cord = visualizer::mapping_phase_to_buffer(
                oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
            buffer.set_pixel(cord.x, cord.y, white);
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    RGFW_surface_free(surface);
    RGFW_window_close(window);
    RGFW_deinit();

    visualizer::output_ppm(buffer, output_path);

    buffer.clear();

    output_path = output_directory / "oscillators_updated_location.ppm";

    oscillator::update_soa_batch(oscillators, 1000);

    for (std::size_t i = 0; i < numbers; ++i) {
        visualizer::Coordnate cord = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
        buffer.set_pixel(cord.x, cord.y, white);
    }

    visualizer::output_ppm(buffer, output_path);
}
