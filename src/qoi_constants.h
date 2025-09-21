#pragma once

#include <qoi_types.h>

#include <array>
#include <cstdint>
#include <string>

namespace qoi {
// QOI INFO
constexpr long long MAX_FILE_SIZE = 1000000000; // 1GB
constexpr std::uint32_t QOI_HEADER_SIZE = 14;   // 14 bytes
constexpr const std::array<Byte, 4> QOI_MAGIC_TAG = {'q', 'o', 'i', 'f'};
constexpr const std::array<Byte, 8> QOI_END_MARKER = {0, 0, 0, 0, 0, 0, 0, 1};

// QOI TAGS
constexpr Byte QOI_OP_RGBA = 0xFF;
constexpr Byte QOI_OP_RGB = 0xFE;
constexpr Byte QOI_OP_INDEX = 0x00;
constexpr Byte QOI_OP_DIFF = 0x40;
constexpr Byte QOI_OP_LUMA = 0x80;
constexpr Byte QOI_OP_RUN = 0xC0;

// PPM INFO
constexpr std::string PPM_MAGIC_TAG = "P6";
constexpr std::uint32_t PPM_MAX_PIXEL_VALUE = 255;

// CLI INFO
constexpr std::string DECODE_OP = "decode";
constexpr std::string ENCODE_OP = "encode";
} // namespace qoi
