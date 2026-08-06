#include <RGFW.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "oscillator_batch.h"
#include "phase_space_renderer.h"
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

}  // namespace visualizer

int main() {
    constexpr double dt = 0.001;
    constexpr int seed = 1234;
    constexpr int oscillator_number = 30000;
    constexpr int width = 1025;
    constexpr int height = 1025;
    constexpr double time_scale = 0.1;

    const fs::path output_directory = fs::path(PROJECT01_SOURCE_DIR) / "results";
    fs::create_directories(output_directory);
    fs::path output_path = output_directory / "oscillators_location.ppm";

    visualizer::PixelBuffer buffer(width, height);
    oscillator::OscillatorSoABatch initial_oscillators =
        oscillator::make_oscillator_soa_batch(oscillator_number, dt, seed);
    // 将相空间纵坐标 v/omega 转换为积分使用的物理速度 v。
    visualizer::convert_phase_velocity_to_physical(initial_oscillators);

    oscillator::OscillatorSoABatch work_oscillators = initial_oscillators;

    visualizer::PhaseSpaceRenderer renderer(work_oscillators, buffer, visualizer::light_blue,
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
        return 1;
    }

    bool paused = false;
    auto previous_time = std::chrono::steady_clock::now();
    // 累积真实时间，按固定 dt 推进整数步。
    double simulation_accumulator = 0.0;
    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_pollEvents();
        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>(' '))) {
            paused = !paused;
        }

        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>('r'))) {
            paused = true;
            work_oscillators = initial_oscillators;
            renderer.render(work_oscillators, buffer, 3);
            RGFW_window_blitSurface(window, surface);
        }

        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>('n'))) {
            paused = true;
            oscillator::update_soa_batch_step(work_oscillators);
            renderer.render(work_oscillators, buffer, 3);
            RGFW_window_blitSurface(window, surface);
        }

        const auto current_time = std::chrono::steady_clock::now();
        const double elapsed_seconds =
            std::chrono::duration<double>(current_time - previous_time).count();
        simulation_accumulator += time_scale * elapsed_seconds;
        previous_time = current_time;
        int steps = simulation_accumulator / dt;
        simulation_accumulator -= steps * dt;

        if (!paused) {
            oscillator::update_soa_batch(work_oscillators, steps);
            renderer.render(work_oscillators, buffer, 3);
            RGFW_window_blitSurface(window, surface);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    RGFW_surface_free(surface);
    RGFW_window_close(window);
}
