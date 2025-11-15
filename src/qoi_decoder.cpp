#include "qoi_types.h"
#include <qoi_decoder.h>

#include <qoi_constants.h>
#include <qoi_utils.h>

#include <stdexcept>

namespace qoi {
// CREATOR
Decoder::Decoder(Offset offset)
    : d_prevPixel{.d_red = 0, .d_green = 0, .d_blue = 0, .d_alpha = 255}, d_pixelCache(64),
      d_outputBuffer(), d_offset(offset) {
    d_outputBuffer.reserve(500000);
}

// MANIPULATOR
DecodedOutput Decoder::decodeQOI(const FileOutput &fileData) {
    while (d_offset < fileData.d_bytes.size() - 8) {
        Pixel currPixel;
        if (fileData.d_bytes[d_offset] == QOI_OP_RGBA) {
            processRGBAOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else if (fileData.d_bytes[d_offset] == QOI_OP_RGB) {
            processRGBOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else if ((fileData.d_bytes[d_offset] >> 6) == (QOI_OP_INDEX >> 6)) {
            processIndexOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else if ((fileData.d_bytes[d_offset] >> 6) == (QOI_OP_DIFF >> 6)) {
            processDiffOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else if ((fileData.d_bytes[d_offset] >> 6) == (QOI_OP_LUMA >> 6)) {
            processLumaOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else if ((fileData.d_bytes[d_offset] >> 6) == (QOI_OP_RUN >> 6)) {
            processRunOp(fileData.d_bytes, currPixel, fileData.d_channels);
        } else {
            throw std::runtime_error("Corrupted byte, qoi operation does not exist for this byte");
        }

        d_pixelCache[hashIndex(currPixel)] = currPixel;
        d_prevPixel = currPixel;
    }

    // Return pixels

    return {.d_width = fileData.d_width,
            .d_height = fileData.d_height,
            .d_channels = fileData.d_channels,
            .d_colorspace = fileData.d_colorspace,
            .d_bytes = d_outputBuffer};
}

void Decoder::processRGBAOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    ++d_offset;
    pixel.d_red = bytes[d_offset++];
    pixel.d_green = bytes[d_offset++];
    pixel.d_blue = bytes[d_offset++];
    pixel.d_alpha = bytes[d_offset++];
    produceToOutBuffer(pixel, channel);
}

void Decoder::processRGBOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    ++d_offset;
    pixel.d_red = bytes[d_offset++];
    pixel.d_green = bytes[d_offset++];
    pixel.d_blue = bytes[d_offset++];
    pixel.d_alpha = d_prevPixel.d_alpha;
    produceToOutBuffer(pixel, channel);
}

void Decoder::processDiffOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    constexpr Byte DIFF_MASK = 0x03;
    int8_t dr = ((bytes[d_offset] >> 4) & DIFF_MASK) - 2;
    int8_t dg = ((bytes[d_offset] >> 2) & DIFF_MASK) - 2;
    int8_t db = (bytes[d_offset] & DIFF_MASK) - 2;

    pixel.d_red = d_prevPixel.d_red + dr;
    pixel.d_green = d_prevPixel.d_green + dg;
    pixel.d_blue = d_prevPixel.d_blue + db;
    pixel.d_alpha = d_prevPixel.d_alpha;
    produceToOutBuffer(pixel, channel);
    ++d_offset;
}

void Decoder::processLumaOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    constexpr Byte GREEN_DIFF_MASK = 0x3F;
    constexpr Byte RED_BLUE_DIFF_MASK = 0x0F;
    int8_t dg = (bytes[d_offset] & GREEN_DIFF_MASK) - 32;
    ++d_offset;
    int8_t dr_dg = ((bytes[d_offset] >> 4) & RED_BLUE_DIFF_MASK) - 8;
    int8_t db_dg = (bytes[d_offset] & RED_BLUE_DIFF_MASK) - 8;
    pixel.d_red = d_prevPixel.d_red + dr_dg + dg;
    pixel.d_green = d_prevPixel.d_green + dg;
    pixel.d_blue = d_prevPixel.d_blue + db_dg + dg;
    pixel.d_alpha = d_prevPixel.d_alpha;
    produceToOutBuffer(pixel, channel);
    ++d_offset;
}

void Decoder::processIndexOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    auto index = static_cast<std::uint32_t>(bytes[d_offset]);
    pixel = d_pixelCache[index];
    produceToOutBuffer(pixel, channel);
    ++d_offset;
}

void Decoder::processRunOp(const Bytes &bytes, Pixel &pixel, Channel channel) {
    constexpr Byte RUN_MASK = 0x3F;
    int8_t runLength = bytes[d_offset] & RUN_MASK;
    pixel = d_prevPixel;
    while (runLength >= 0) {
        produceToOutBuffer(pixel, channel);
        --runLength;
    }
    ++d_offset;
}

void Decoder::produceToOutBuffer(const Pixel &pixel, Channel channel) {
    d_outputBuffer.emplace_back(pixel.d_red);
    d_outputBuffer.emplace_back(pixel.d_green);
    d_outputBuffer.emplace_back(pixel.d_blue);

    if (channel == 4) {
        d_outputBuffer.emplace_back(pixel.d_alpha);
    }
}
} // namespace qoi
