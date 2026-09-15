#pragma once
#include <cstdint>
#include <vector>

#include "pixel_buffer.h"
#include "state.h"

namespace visualizer {

struct PixelAccumulator {
    std::uint32_t sum_r{};
    std::uint32_t sum_g{};
    std::uint32_t sum_b{};
    std::uint32_t count{};
};

struct TouchedPixels {
    std::size_t pixel;
    int life;
};

void convert_phase_velocity_to_physical(oscillator::OscillatorSoABatch& oscillators);

class PhaseSpaceRenderer {
   public:
    PhaseSpaceRenderer(const oscillator::OscillatorSoABatch& oscillators, PixelBuffer& buffer,
                       const Rgba8& low_color, const Rgba8& mid_color, const Rgba8& high_color);

    void render(const oscillator::OscillatorSoABatch& oscillators, PixelBuffer& buffer,
                const double reference_count);

    void render(const oscillator::OscillatorSoABatch& oscillators, PixelBuffer& buffer,
                const double reference_count, const int generations);

   private:
    std::vector<PixelAccumulator> accumulators_;
    std::vector<std::size_t> touched_pixels_;
    std::vector<TouchedPixels> touched_pixels2_;
    std::vector<int> remainint_life_;
    std::vector<std::size_t> active_pixels_;
    std::vector<Rgba8> oscillator_colors_;
};

}  // namespace visualizer
