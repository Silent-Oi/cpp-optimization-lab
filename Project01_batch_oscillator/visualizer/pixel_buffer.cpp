#include "pixel_buffer.h"
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace visualizer {

PixelBuffer::PixelBuffer(int width, int height)
    : width_(width), height_(height){
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("窗口大小必须为正整数");
    }
    bytes_ = std::vector<std::uint8_t>(width * height * 4, 0);

    for (int i = 0; i < width * height; ++i) {
        int index = 4 * i;
        bytes_[index + 3] = 255;
    }
}

int PixelBuffer::width() const {
    return width_;
}

int PixelBuffer::height() const {
    return height_;
}

const std::vector<std::uint8_t>& PixelBuffer::bytes() const {
    return bytes_;
}

bool PixelBuffer::set_pixel(int x, int y, Rgba8 color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return false;
    }

    std::size_t index = (static_cast<std::size_t>(width_) * static_cast<std::size_t>(y) +
                         static_cast<std::size_t>(x)) *
                        4;
    bytes_[index] = color.r;
    bytes_[index + 1] = color.g;
    bytes_[index + 2] = color.b;
    bytes_[index + 3] = color.a;

    return true;
}

void PixelBuffer::clear() {
    for (int i = 0; i < width_ * height_; ++i) {
        int index = 4 * i;
        bytes_[index] = 0;
        bytes_[index + 1] = 0;
        bytes_[index + 2] = 0;
        bytes_[index + 3] = 255;
    }
}

}  // namespace visualizer
