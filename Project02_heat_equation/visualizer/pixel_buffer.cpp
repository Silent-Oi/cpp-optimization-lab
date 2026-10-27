#include "pixel_buffer.h"

#include <cstdint>
#include <stdexcept>
#include <vector>
namespace visualizer {

PixelBuffer::PixelBuffer(std::size_t width, std::size_t height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("窗口大小必须为正整数");
    }
    // RGBA8 每个像素连续占 4 字节。
    pixel_count_ = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    bytes_ = std::vector<std::uint8_t>(pixel_count_ * 4, 0);
    for (std::size_t pixel = 0; pixel < pixel_count_; ++pixel) {
        const std::size_t byte_index = pixel * 4;
        bytes_[byte_index + 3] = 255;
    }
}

std::size_t PixelBuffer::width() const {
    return width_;
}

std::size_t PixelBuffer::height() const {
    return height_;
}

std::size_t PixelBuffer::pixel_count() const {
    return pixel_count_;
}

const std::vector<std::uint8_t>& PixelBuffer::bytes() const {
    return bytes_;
}

std::vector<std::uint8_t>& PixelBuffer::bytes() {
    return bytes_;
}

std::uint8_t* PixelBuffer::data() {
    return bytes_.data();
}

bool PixelBuffer::set_pixel(std::size_t x, std::size_t y, Rgba8 color) {
    if (x >= width_ || y >= height_) {
        return false;
    }

    const std::size_t pixel = (static_cast<std::size_t>(width_) * static_cast<std::size_t>(y) +
                               static_cast<std::size_t>(x));

    const std::size_t byte_index = pixel * 4;
    bytes_[byte_index] = color.r;
    bytes_[byte_index + 1] = color.g;
    bytes_[byte_index + 2] = color.b;
    bytes_[byte_index + 3] = color.a;

    return true;
}

bool PixelBuffer::set_pixel(std::size_t pixel, Rgba8 color) {
    if (pixel >= pixel_count_) {
        return false;
    }

    const std::size_t byte_index = pixel * 4;

    bytes_[byte_index] = color.r;
    bytes_[byte_index + 1] = color.g;
    bytes_[byte_index + 2] = color.b;
    bytes_[byte_index + 3] = color.a;
    return true;
}

void PixelBuffer::clear() {
    for (std::size_t pixel = 0; pixel < pixel_count_; ++pixel) {
        const std::size_t byte_index = pixel * 4;
        bytes_[byte_index] = 0;
        bytes_[byte_index + 1] = 0;
        bytes_[byte_index + 2] = 0;
        bytes_[byte_index + 3] = 255;
    }
}

}  // namespace visualizer
