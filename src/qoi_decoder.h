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
    Pixels d_outputBuffer;
    Offset d_offset;

    // PRIVATE MANIPULATORS
    void processRGBAOp(const Bytes &bytes, Pixel &pixel);

    void processRGBOp(const Bytes &bytes, Pixel &pixel);

    void processDiffOp(const Bytes &bytes, Pixel &pixel);

    void processLumaOp(const Bytes &bytes, Pixel &pixel);

    void processIndexOp(const Bytes &bytes, Pixel &pixel);

    void processRunOp(const Bytes &bytes, Pixel &pixel);

  public:
    // CREATOR
    Decoder(Offset offset = 0);

    // MANIPULATORS
    DecodedOutput decodeQOI(const FileOutput &fileData);
};

} // namespace qoi
