#include <qoi_encoder.h>

#include <qoi_constants.h>
#include <qoi_types.h>
#include <qoi_utils.h>

namespace qoi {
// CREATORS
Encoder::Encoder()
    : d_prevPixel{.d_red = 0, .d_green = 0, .d_blue = 0, .d_alpha = 255}, d_pixelCache(64),
      d_encodedBuffer(), d_run{0} {
    d_encodedBuffer.reserve(10000000);
}

// MANIPULATORS
EncodedOutput Encoder::encodeToQOI(const FileOutput &fileData) {
    Pixels pixels(fileData.d_height * fileData.d_width);

    convertBytesToPixel(fileData.d_bytes, pixels);

    // write header
    d_encodedBuffer.insert(d_encodedBuffer.cend(), QOI_MAGIC_TAG.cbegin(), QOI_MAGIC_TAG.cend());

    // write width
    writeU32(fileData.d_width, d_encodedBuffer);

    // write height
    writeU32(fileData.d_height, d_encodedBuffer);

    // write channels
    d_encodedBuffer.emplace_back(fileData.d_channels);

    // write colorspace
    d_encodedBuffer.emplace_back(fileData.d_colorspace);

    // write pixels data
    for (auto iter = pixels.begin(); iter != pixels.end(); iter++) {
        if (*iter == d_prevPixel) {
            ++d_run;
            if (d_run == 62 || iter == pixels.end() - 1) {
                d_encodedBuffer.emplace_back(QOI_OP_RUN | d_run - 1);
                d_run = 0;
            }

            continue;
        }

        if (d_run > 0) {
            d_encodedBuffer.emplace_back(QOI_OP_RUN | d_run - 1);
            d_run = 0;
        }

        int index = hashIndex(*iter);
        if (d_pixelCache[index] == *iter) {
            d_encodedBuffer.emplace_back(QOI_OP_INDEX | index);
        } else {
            int dr = iter->d_red - d_prevPixel.d_red;
            int dg = iter->d_green - d_prevPixel.d_green;
            int db = iter->d_blue - d_prevPixel.d_blue;

            int dr_dg = dr - dg;
            int db_dg = db - dg;

            if ((dr >= -2 && dr <= 1) && (dg >= -2 && dg <= 1) && (db >= -2 && db <= 1)) {
                d_encodedBuffer.emplace_back(QOI_OP_DIFF | ((dr + 2) << 4) | ((dg + 2) << 2) |
                                             (db + 2));
            } else if ((dg >= -32 && dg <= 31) && (dr_dg >= -8 && dr_dg <= 7) &&
                       (db_dg >= -8 && db_dg <= 7)) {
                d_encodedBuffer.emplace_back(QOI_OP_LUMA | (dg + 32));
                d_encodedBuffer.emplace_back(((dr_dg + 8) << 4) | (db_dg + 8));
            } else if (iter->d_alpha == d_prevPixel.d_alpha) {
                d_encodedBuffer.emplace_back(QOI_OP_RGB);
                d_encodedBuffer.emplace_back(iter->d_red);
                d_encodedBuffer.emplace_back(iter->d_green);
                d_encodedBuffer.emplace_back(iter->d_blue);
            } else {
                d_encodedBuffer.emplace_back(QOI_OP_RGBA);
                d_encodedBuffer.emplace_back(iter->d_red);
                d_encodedBuffer.emplace_back(iter->d_green);
                d_encodedBuffer.emplace_back(iter->d_blue);
                d_encodedBuffer.emplace_back(iter->d_alpha);
            }
        }

        d_pixelCache[index] = *iter;
        d_prevPixel = *iter;
    }

    // write end marker
    d_encodedBuffer.insert(d_encodedBuffer.end(), QOI_END_MARKER.cbegin(), QOI_END_MARKER.cend());

    return {.d_bytes = d_encodedBuffer};
}
} // namespace qoi
