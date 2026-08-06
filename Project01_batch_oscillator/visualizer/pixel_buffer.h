#pragma once
#include <cstdint>
#include <vector>

namespace visualizer {
struct Rgba8 {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;
};

struct Coordinate {
    int x;
    int y;
};

struct PixelAccumulator {
    std::uint32_t sum_r{};
    std::uint32_t sum_g{};
    std::uint32_t sum_b{};
    std::uint32_t count{};
};

class PixelBuffer {
   public:
    PixelBuffer(int width, int height);

    int width() const;
    int height() const;
    const std::vector<std::uint8_t>& bytes() const;

    std::uint8_t* data();

    bool set_pixel(int x, int y, Rgba8 color);
    void clear();

   private:
    int width_;
    int height_;
    std::vector<std::uint8_t> bytes_;
};

}  // namespace visualizer
