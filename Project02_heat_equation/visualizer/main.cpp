#include <RGFW.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <thread>
#include <cstdio>
#include "heat_diffusion.h"
#include "heatmap_renderer.h"
#include "pixel_buffer.h"
#include "state.h"

namespace {

const visualizer::Rgba8 blue{0, 180, 255, 255};     // 青蓝
const visualizer::Rgba8 light{220, 240, 255, 255};  // 亮白蓝
const visualizer::Rgba8 gold{255, 220, 80, 255};    // 金黄
const visualizer::Rgba8 white{255, 255, 255, 255};
const visualizer::Rgba8 black{0, 0, 0, 255};
const visualizer::Rgba8 light_blue{100, 190, 255, 255};
const visualizer::Rgba8 mid_white{245, 245, 245, 255};  // 暖白：中阻尼
const visualizer::Rgba8 light_red{255, 125, 135, 255};  // 浅红：高阻尼

const heat_diffusion::Grid2D default_grid{
    512,
    512,
    1.0 / 511,
    1.0 / 511,
};
const heat_diffusion::InitialField default_field_type = heat_diffusion::InitialField::Uniform;
const double initial_temperature = -64;
const double thermal_diffusivity = 1e-5;
const double dt = 0.01;

constexpr int buffer_width = 1024;
constexpr int buffer_height = 1024;
visualizer::PixelBuffer buffer(buffer_width, buffer_height);
std::int32_t mx = 0;
std::int32_t my = 0;
RGFW_event event;

constexpr double real_simul_time_scale = 100;

}  // namespace

int main() {
    heat_diffusion::TemperatureField initial_field(default_grid, default_field_type,
                                                   initial_temperature);
    heat_diffusion::TemperatureField work_field = initial_field;

    heat_diffusion::NuemricalWorkspace workspace =
        work_field.make_numerical_workspace(thermal_diffusivity, dt);

    RGFW_window* window = RGFW_createWindow("heat_diffusion Visualizer", 0, 0, buffer_width,
                                            buffer_height, RGFW_windowCenter | RGFW_windowNoResize);
    if (window == nullptr) {
        RGFW_deinit();
        return 1;
    }
    RGFW_window_setExitKey(window, RGFW_escape);
    RGFW_surface* surface =
        RGFW_createSurface(buffer.data(), buffer_width, buffer_height, RGFW_formatRGBA8);
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
            work_field = initial_field;
            map_grid_to_buffer(work_field, buffer, std::abs(initial_temperature), light_red,
                               light_blue);
            RGFW_window_blitSurface(window, surface);
        }

        if (RGFW_window_isKeyPressed(window, static_cast<RGFW_key>('n'))) {
            paused = true;
            work_field.numerical_solve_step(workspace);
            work_field.get_temperature().swap(workspace.temperature_cache);
            map_grid_to_buffer(work_field, buffer, std::abs(initial_temperature), light_red,
                               light_blue);
            RGFW_window_blitSurface(window, surface);
        }

        const auto current_time = std::chrono::steady_clock::now();
        const double elapsed_seconds =
            std::chrono::duration<double>(current_time - previous_time).count();
        simulation_accumulator += real_simul_time_scale * elapsed_seconds;
        previous_time = current_time;
        int steps = simulation_accumulator / dt;
        simulation_accumulator -= steps * dt;
        //printf("%d\n", elapsed_seconds);

        if (!paused) {
            work_field.numerical_solve_parallel(workspace, steps, 4);
            map_grid_to_buffer(work_field, buffer, std::abs(initial_temperature), light_red,
                               light_blue);
            RGFW_window_blitSurface(window, surface);
            visualizer::test_mouse_position(window,event,mx,my);
            visualizer::apply_heat_brush(window, event, mx, my, work_field, buffer, 1000000000.0,
                                         real_simul_time_scale * elapsed_seconds,
                                         thermal_diffusivity, 1);
            ;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    RGFW_surface_free(surface);
    RGFW_window_close(window);
}
