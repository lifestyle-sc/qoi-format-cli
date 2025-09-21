#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace qoi {
using Byte = std::uint8_t;
using ColorSpace = std::uint8_t;
using Channel = std::uint8_t;
using Height = std::uint32_t;
using Width = std::uint32_t;
using Hash = std::uint32_t;

struct QOIHeader {
    std::array<Byte, 4> d_magic;
    Width d_width;
    Height d_height;
    Channel d_channels;
    ColorSpace d_colorspace;
};

struct Pixel {
    Byte d_red{0};
    Byte d_green{0};
    Byte d_blue{0};
    Byte d_alpha{0};

    bool operator==(const Pixel &rhs) const {
        return (d_red == rhs.d_red) && (d_green == rhs.d_green) && (d_blue == rhs.d_blue) &&
               (d_alpha == rhs.d_alpha);
    }
};

struct FileOutput {
    Width d_width;
    Height d_height;
    Channel d_channels;
    ColorSpace d_colorspace;
    std::vector<Byte> d_bytes;
};

struct EncodedOutput {
    std::vector<Byte> d_bytes;
};

struct DecodedOutput {
    Width d_width;
    Height d_height;
    Channel d_channels;
    ColorSpace d_colorspace;
    std::vector<Pixel> d_pixels;
};

} // namespace qoi
