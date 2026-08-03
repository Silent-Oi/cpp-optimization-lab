#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
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
static Corrdnate mapping_phase_to_buffer(const double position, const double velocity,
                                         const PixelBuffer& buffer) {
    int center_x = buffer.width() * 0.5;
    int center_y = buffer.height() * 0.5;

    int cord_x = static_cast<int>(position * center_x) + center_x;
    int cord_y = static_cast<int>(-1 * velocity * center_y) + center_y;

    if (cord_x >= buffer.width() || cord_x < 0 || cord_y >= buffer.height() || cord_y < 0)
    {
        return {-1, -1};

    }
    return {cord_x,
            cord_y};
}

static void norm_velocity(oscillator::OscillatorSoABatch& oscillators)
{
    std::size_t numbers = oscillators.omega.size();
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

    int width = 1025;
    int heigth = 1025;

    visualizer::PixelBuffer buffer(1025, 1025);
    

    oscillator::OscillatorSoABatch oscillators =
        oscillator::make_oscillator_soa_batch(50000, 0.01, 1234);
    visualizer::norm_velocity(oscillators);

    std::size_t numbers = oscillators.omega.size();

    for (std::size_t i = 0; i < numbers; ++i) {
        visualizer::Corrdnate cord = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
        buffer.set_pixel(cord.x, cord.y, white);
    }

    visualizer::output_ppm(buffer, output_path);


    buffer.clear();

    output_path = output_directory / "oscillators_updated_location.ppm";

    oscillator::update_soa_batch(oscillators, 1000);

    for (std::size_t i = 0; i < numbers; ++i) {
        visualizer::Corrdnate cord = visualizer::mapping_phase_to_buffer(
            oscillators.position[i], oscillators.velocity[i] / oscillators.omega[i], buffer);
        buffer.set_pixel(cord.x, cord.y, white);
    }

    visualizer::output_ppm(buffer, output_path);
    
}
