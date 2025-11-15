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
    const auto pixelChannels = static_cast<std::size_t>(fileData.d_channels);
    if (pixelChannels < 3) {
        throw std::runtime_error("unsupported channel size, support only 3 or 4 channels!");
    }

    if (fileData.d_bytes.size() % pixelChannels != 0) {
        throw std::runtime_error("The data is corrupted or incomplete");
    }

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
    const auto &bytes = fileData.d_bytes;
    for (auto iter = 0; iter != bytes.size(); iter += pixelChannels) {
        auto currPixel = Pixel{.d_red = bytes[iter],
                               .d_green = bytes[iter + 1],
                               .d_blue = bytes[iter + 2],
                               .d_alpha = 255};
        if (pixelChannels == 4) {
            currPixel.d_alpha = bytes[iter + 3];
        }

        if (currPixel == d_prevPixel) {
            ++d_run;
            if (d_run == 62) {
                d_encodedBuffer.emplace_back(QOI_OP_RUN | d_run - 1);
                d_run = 0;
            }

            continue;
        }

        if (d_run > 0) {
            d_encodedBuffer.emplace_back(QOI_OP_RUN | d_run - 1);
            d_run = 0;
        }

        int index = hashIndex(currPixel);
        if (d_pixelCache[index] == currPixel) {
            d_encodedBuffer.emplace_back(QOI_OP_INDEX | index);
        } else if (currPixel.d_alpha == d_prevPixel.d_alpha) {
            int dr = currPixel.d_red - d_prevPixel.d_red;
            int dg = currPixel.d_green - d_prevPixel.d_green;
            int db = currPixel.d_blue - d_prevPixel.d_blue;

            int dr_dg = dr - dg;
            int db_dg = db - dg;

            if ((dr >= -2 && dr <= 1) && (dg >= -2 && dg <= 1) && (db >= -2 && db <= 1)) {
                d_encodedBuffer.emplace_back(QOI_OP_DIFF | ((dr + 2) << 4) | ((dg + 2) << 2) |
                                             (db + 2));
            } else if ((dg >= -32 && dg <= 31) && (dr_dg >= -8 && dr_dg <= 7) &&
                       (db_dg >= -8 && db_dg <= 7)) {
                d_encodedBuffer.emplace_back(QOI_OP_LUMA | (dg + 32));
                d_encodedBuffer.emplace_back(((dr_dg + 8) << 4) | (db_dg + 8));
            } else {
                d_encodedBuffer.emplace_back(QOI_OP_RGB);
                d_encodedBuffer.emplace_back(currPixel.d_red);
                d_encodedBuffer.emplace_back(currPixel.d_green);
                d_encodedBuffer.emplace_back(currPixel.d_blue);
            }
        } else {
            d_encodedBuffer.emplace_back(QOI_OP_RGBA);
            d_encodedBuffer.emplace_back(currPixel.d_red);
            d_encodedBuffer.emplace_back(currPixel.d_green);
            d_encodedBuffer.emplace_back(currPixel.d_blue);
            d_encodedBuffer.emplace_back(currPixel.d_alpha);
        }

        d_pixelCache[index] = currPixel;
        d_prevPixel = currPixel;
    }

    if (d_run > 0) {
        d_encodedBuffer.emplace_back(QOI_OP_RUN | d_run - 1);
        d_run = 0;
    }

    // write end marker
    d_encodedBuffer.insert(d_encodedBuffer.end(), QOI_END_MARKER.cbegin(), QOI_END_MARKER.cend());

    return {.d_bytes = d_encodedBuffer};
}
} // namespace qoi
