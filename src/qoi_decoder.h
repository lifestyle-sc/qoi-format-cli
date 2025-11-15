#pragma once

#include <qoi_types.h>

#include <cstddef>
#include <vector>

namespace qoi {
class Decoder {
    // TYPES
    using Pixels = std::vector<Pixel>;
    using Bytes = std::vector<Byte>;
    using Offset = std::size_t;

    // DATA
    Pixel d_prevPixel;
    Pixels d_pixelCache;
    Bytes d_outputBuffer;
    Offset d_offset;

    // PRIVATE MANIPULATORS
    void processRGBAOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void processRGBOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void processDiffOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void processLumaOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void processIndexOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void processRunOp(const Bytes &bytes, Pixel &pixel, Channel channel);

    void produceToOutBuffer(const Pixel &pixel, Channel channel);

  public:
    // CREATOR
    Decoder(Offset offset = 0);

    // MANIPULATORS
    DecodedOutput decodeQOI(const FileOutput &fileData);
};

} // namespace qoi
