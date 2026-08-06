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

const Rgba8 blue{0, 180, 255, 255};     // 青蓝
const Rgba8 light{220, 240, 255, 255};  // 亮白蓝
const Rgba8 gold{255, 220, 80, 255};    // 金黄
const Rgba8 white{255, 255, 255, 255};
const Rgba8 black{0, 0, 0, 255};
const Rgba8 light_blue{100, 190, 255, 255};
const Rgba8 mid_white{245, 245, 245, 255};  // 暖白：中阻尼
const Rgba8 light_red{255, 125, 135, 255};  // 浅红：高阻尼

struct Coordinate {
    int x;
    int y;
};

class PixelBuffer {
   public:
    PixelBuffer(int width, int height);

    int width() const;
    int height() const;
    std::size_t pixel_count() const;
    const std::vector<std::uint8_t>& bytes() const;

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
