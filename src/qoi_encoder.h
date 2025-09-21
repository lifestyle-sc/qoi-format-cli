#pragma once

#include <qoi_types.h>

#include <cstddef>
#include <vector>

namespace qoi {
class Encoder {
    // TYPES
    using Pixels = std::vector<Pixel>;
    using Bytes = std::vector<Byte>;

    // DATA
    Pixel d_prevPixel;
    Pixels d_pixelCache;
    Bytes d_encodedBuffer;
    int d_run;

  public:
    // CREATORS
    Encoder();

    // MANIPULATORS
    EncodedOutput encodeToQOI(const FileOutput &fileData);
};
} // namespace qoi
