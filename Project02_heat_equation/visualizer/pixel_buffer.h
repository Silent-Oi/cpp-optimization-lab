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

class PixelBuffer {
   public:
    PixelBuffer(int width, int height);

    int width() const;
    int height() const;
    std::size_t pixel_count() const;
    const std::vector<std::uint8_t>& bytes() const;
    std::vector<std::uint8_t>& bytes();

    std::uint8_t* data();

    bool set_pixel(int x, int y, Rgba8 color);
    bool set_pixel(std::size_t pixel, Rgba8 color);
    void clear();

   private:
    int width_;
    int height_;
    std::size_t pixel_number_;
    std::vector<std::uint8_t> bytes_;
};



}  // namespace visualizer
