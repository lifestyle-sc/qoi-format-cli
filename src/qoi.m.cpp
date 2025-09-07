#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Byte = std::uint8_t;

constexpr long long MAX_FILE_SIZE = 1000000000; // 1GB
constexpr std::uint32_t QOI_HEADER_SIZE = 14;   // 14 bytes
constexpr const std::array<Byte, 4> QOI_MAGIC_TAG = {'q', 'o', 'i', 'f'};
constexpr const std::array<Byte, 8> QOI_END_MARKER = {0, 0, 0, 0, 0, 0, 0, 1};

// TAGS
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

auto byte_ptr(char *p) -> Byte * { return reinterpret_cast<Byte *>(p); }
auto char_ptr(Byte *p) -> char * { return reinterpret_cast<char *>(p); }

struct QOIHeader {
    std::array<Byte, 4> d_magic;
    std::uint32_t d_width;
    std::uint32_t d_height;
    std::uint8_t d_channels;
    std::uint8_t d_colorspace;
};

struct RGBA {
    std::uint8_t d_red{0};
    std::uint8_t d_green{0};
    std::uint8_t d_blue{0};
    std::uint8_t d_alpha{0};

    bool operator==(const RGBA &rhs) const {
        return (d_red == rhs.d_red) && (d_green == rhs.d_green) && (d_blue == rhs.d_blue) &&
               (d_alpha == rhs.d_alpha);
    }
};

struct DecodedOutput {
    QOIHeader d_header;
    std::vector<RGBA> d_pixels;
};

struct PPMOutput {
    std::uint32_t d_width;
    std::uint32_t d_height;
    std::uint8_t d_channels;
    std::uint8_t d_colorspace;
    std::vector<RGBA> d_pixels;
};

auto hashIndex(const RGBA &pixel) -> std::uint32_t {
    return ((pixel.d_red * 3) + (pixel.d_green * 5) + (pixel.d_blue * 7) + (pixel.d_alpha * 11)) %
           64;
}

constexpr std::vector<Byte> readQOIFile(const std::filesystem::path &filename) {
    std::ifstream file{filename, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename.string());
    }

    std::streamsize file_size = static_cast<std::streamsize>(std::filesystem::file_size(filename));

    if (file_size > MAX_FILE_SIZE) {
        throw std::runtime_error(filename.string() + " exceeds the limit of 1GB");
    }

    std::vector<Byte> buffer(file_size);

    if (!file.read(char_ptr(buffer.data()), file_size)) {
        throw std::runtime_error("Failed to read the full contents of " + filename.string());
    }

    return buffer;
}

constexpr PPMOutput readPPMFile(const std::filesystem::path &filename) {
    std::ifstream file{filename, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename.string());
    }

    std::string tag;
    file >> tag;
    if (tag != PPM_MAGIC_TAG) {
        throw std::runtime_error("Unsupported PPM format. Only P6 (binary RGB) is supported.");
    }
    std::cout << "Magic Tag: " << tag << std::endl;

    // Skip comment line
    char peekChar = file.peek();
    while (std::isspace(peekChar)) {
        file.get();
        peekChar = file.peek();
    }

    while (peekChar == '#') {
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        peekChar = file.peek();

        while (std::isspace(peekChar)) {
            file.get();
            peekChar = file.peek();
        }
    }

    std::uint32_t width, height, maxPixelValue;
    file >> width >> height >> maxPixelValue;
    std::cout << "PPM Width: " << width << ", Height: " << height
              << ", and max pixel value: " << maxPixelValue << std::endl;

    if (maxPixelValue != PPM_MAX_PIXEL_VALUE) {
        throw std::runtime_error("Only 8-bit PPM files (max color 255) are supported.");
    }

    file.ignore();

    std::vector<Byte> pixelBuffer(width * height * 3);

    file.read(reinterpret_cast<char *>(pixelBuffer.data()), pixelBuffer.size());
    if (!file) {
        std::cerr << "Unable to read pixel data from PPM file" << std::endl;
        throw std::runtime_error("Unable to read pixel data from PPM file");
    }

    std::vector<RGBA> pixels(width * height);
    for (int iter = 0; iter < pixels.size(); ++iter) {
        pixels[iter] = {.d_red = pixelBuffer[iter * 3],
                        .d_green = pixelBuffer[iter * 3 + 1],
                        .d_blue = pixelBuffer[iter * 3 + 2],
                        .d_alpha = 255};
    }

    return {.d_width = width,
            .d_height = height,
            .d_channels = 3,
            .d_colorspace = 0,
            .d_pixels = pixels};
}

void printByte(Byte byte) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ' ';
}

void printBuffer(const std::vector<Byte> &buffer) {
    for (auto &byte : buffer) {
        printByte(byte);
    }

    std::cout << std::dec << std::endl; // reset stream to decimal
}

void printMagicTag(const std::array<Byte, 4> &magicTag) {
    std::cout << "Magic tag: ";
    for (auto &ch : magicTag) {
        std::cout << ch;
    }

    std::cout << std::endl;
}

void printPixel(const RGBA &pixel) {
    std::cout << "RED: " << static_cast<uint32_t>(pixel.d_red)
              << ", GREEN: " << static_cast<uint32_t>(pixel.d_green)
              << ", BLUE: " << static_cast<uint32_t>(pixel.d_blue)
              << ", ALPHA: " << static_cast<uint32_t>(pixel.d_alpha) << std::endl;
}

std::uint32_t readU32(const std::vector<Byte> &buffer, std::size_t &offset) {
    uint32_t red = buffer[offset++] << 24;
    uint32_t blue = buffer[offset++] << 16;
    uint32_t green = buffer[offset++] << 8;
    uint32_t alpha = buffer[offset++];
    return (red | blue | green | alpha);
}

void writeU32(std::uint32_t value, std::vector<Byte> &encodedBuffer) {
    encodedBuffer.emplace_back((value >> 24) & 0xFF);
    encodedBuffer.emplace_back((value >> 16) & 0xFF);
    encodedBuffer.emplace_back((value >> 8) & 0xFF);
    encodedBuffer.emplace_back(value & 0xFF);
}

void extractHeader(const std::vector<Byte> &buffer, QOIHeader &header, std::size_t &offset) {
    // extract the magic number
    std::copy(buffer.begin(), buffer.begin() + 4, header.d_magic.begin());
    offset += 4;

    // extract width
    header.d_width = readU32(buffer, offset);

    // extract height
    header.d_height = readU32(buffer, offset);

    // extract channels
    header.d_channels = buffer[offset++];

    // extract colorspace
    header.d_colorspace = buffer[offset++];
}

bool hasValidEndMarker(const std::vector<Byte> &buffer) {
    if (buffer.size() < 8) {
        return false;
    }

    int start = buffer.size() - 8;
    for (int i = 0; i < 8; ++i) {
        if (buffer[start + i] != QOI_END_MARKER[i]) {
            return false;
        }
    }

    return true;
}

DecodedOutput decodeQOI(const std::vector<Byte> &buffer) {
    if (buffer.size() < QOI_HEADER_SIZE) {
        throw std::runtime_error("QOI file header is missing!");
    }

    std::size_t offset = 0;

    QOIHeader header{};
    extractHeader(buffer, header, offset);

    printMagicTag(header.d_magic);

    std::cout << "Width: " << header.d_width << "\n"
              << "Height: " << header.d_height << "\n"
              << "Channels: " << static_cast<unsigned int>(header.d_channels) << "\n"
              << "Colorspace: " << static_cast<unsigned int>(header.d_colorspace) << "\n";

    // Extract data chunks
    if (!hasValidEndMarker(buffer)) {
        throw std::runtime_error("QOI file is corrupted or incomplete");
    }

    std::vector<RGBA> pixels(64);
    RGBA prevPixel{.d_red = 0, .d_green = 0, .d_blue = 0, .d_alpha = 255};

    std::vector<RGBA> output;
    output.reserve(500000);

    while (offset < buffer.size() - 8) {
        RGBA currPixel;
        if (buffer[offset] == QOI_OP_RGBA) {
            ++offset;
            currPixel.d_red = buffer[offset++];
            currPixel.d_green = buffer[offset++];
            currPixel.d_blue = buffer[offset++];
            currPixel.d_alpha = buffer[offset++];
            output.emplace_back(currPixel);
        } else if (buffer[offset] == QOI_OP_RGB) {
            ++offset;
            currPixel.d_red = buffer[offset++];
            currPixel.d_green = buffer[offset++];
            currPixel.d_blue = buffer[offset++];
            currPixel.d_alpha = prevPixel.d_alpha;
            output.emplace_back(currPixel);
        } else if ((buffer[offset] >> 6) == (QOI_OP_INDEX >> 6)) {
            auto index = static_cast<std::uint32_t>(buffer[offset]);
            currPixel = pixels[index];
            output.emplace_back(currPixel);
            ++offset;
        } else if ((buffer[offset] >> 6) == (QOI_OP_DIFF >> 6)) {
            constexpr Byte DIFF_MASK = 0x03;
            int8_t dr = ((buffer[offset] >> 4) & DIFF_MASK) - 2;
            int8_t dg = ((buffer[offset] >> 2) & DIFF_MASK) - 2;
            int8_t db = (buffer[offset] & DIFF_MASK) - 2;

            currPixel.d_red = prevPixel.d_red + dr;
            currPixel.d_green = prevPixel.d_green + dg;
            currPixel.d_blue = prevPixel.d_blue + db;
            currPixel.d_alpha = prevPixel.d_alpha;
            output.emplace_back(currPixel);
            ++offset;
        } else if ((buffer[offset] >> 6) == (QOI_OP_LUMA >> 6)) {
            constexpr Byte GREEN_DIFF_MASK = 0x3F;
            constexpr Byte RED_BLUE_DIFF_MASK = 0x0F;
            int8_t dg = (buffer[offset] & GREEN_DIFF_MASK) - 32;
            ++offset;
            int8_t dr_dg = ((buffer[offset] >> 4) & RED_BLUE_DIFF_MASK) - 8;
            int8_t db_dg = (buffer[offset] & RED_BLUE_DIFF_MASK) - 8;
            currPixel.d_red = prevPixel.d_red + dr_dg + dg;
            currPixel.d_green = prevPixel.d_green + dg;
            currPixel.d_blue = prevPixel.d_blue + db_dg + dg;
            currPixel.d_alpha = prevPixel.d_alpha;
            output.emplace_back(currPixel);
            ++offset;
        } else if ((buffer[offset] >> 6) == (QOI_OP_RUN >> 6)) {
            constexpr Byte RUN_MASK = 0x3F;
            int8_t runLength = buffer[offset] & RUN_MASK;
            currPixel = prevPixel;
            while (runLength >= 0) {
                output.emplace_back(currPixel);
                --runLength;
            }
            ++offset;
        }

        pixels[hashIndex(currPixel)] = currPixel;
        prevPixel = currPixel;
    }

    // Return pixels

    return {.d_header = header, .d_pixels = output};
}

std::vector<Byte> encodeToQOI(const PPMOutput &ppmData) {
    std::vector<Byte> encodedData;
    encodedData.reserve(1000);

    // write header
    encodedData.insert(encodedData.cend(), QOI_MAGIC_TAG.cbegin(), QOI_MAGIC_TAG.cend());

    // write width
    writeU32(ppmData.d_width, encodedData);

    // write height
    writeU32(ppmData.d_height, encodedData);

    // write channels
    encodedData.emplace_back(ppmData.d_channels);

    // write colorspace
    encodedData.emplace_back(ppmData.d_colorspace);

    // write pixels data
    std::vector<RGBA> indexArr(64);
    int run = 0;
    RGBA prevPixel{.d_red = 0, .d_green = 0, .d_blue = 0, .d_alpha = 255};

    for (auto iter = ppmData.d_pixels.begin(); iter != ppmData.d_pixels.end(); iter++) {
        if (*iter == prevPixel) {
            ++run;
            if (run == 62 || iter == ppmData.d_pixels.end() - 1) {
                encodedData.emplace_back(QOI_OP_RUN | run - 1);
                run = 0;
            }

            continue;
        }

        if (run > 0) {
            encodedData.emplace_back(QOI_OP_RUN | run - 1);
            run = 0;
        }

        int index = hashIndex(*iter);
        if (indexArr[index] == *iter) {
            encodedData.emplace_back(QOI_OP_INDEX | index);
        } else {
            int dr = iter->d_red - prevPixel.d_red;
            int dg = iter->d_green - prevPixel.d_green;
            int db = iter->d_blue - prevPixel.d_blue;

            int dr_dg = dr - dg;
            int db_dg = db - dg;

            if ((dr >= -2 && dr <= 1) && (dg >= -2 && dg <= 1) && (db >= -2 && db <= 1)) {
                encodedData.emplace_back(QOI_OP_DIFF | ((dr + 2) << 4) | ((dg + 2) << 2) |
                                         (db + 2));
            } else if ((dg >= -32 && dg <= 31) && (dr_dg >= -8 && dr_dg <= 7) &&
                       (db_dg >= -8 && db_dg <= 7)) {
                encodedData.emplace_back(QOI_OP_LUMA | (dg + 32));
                encodedData.emplace_back(((dr_dg + 8) << 4) | (db_dg + 8));
            } else if (iter->d_alpha == prevPixel.d_alpha) {
                encodedData.emplace_back(QOI_OP_RGB);
                encodedData.emplace_back(iter->d_red);
                encodedData.emplace_back(iter->d_green);
                encodedData.emplace_back(iter->d_blue);
            } else {
                encodedData.emplace_back(QOI_OP_RGBA);
                encodedData.emplace_back(iter->d_red);
                encodedData.emplace_back(iter->d_green);
                encodedData.emplace_back(iter->d_blue);
                encodedData.emplace_back(iter->d_alpha);
            }
        }

        indexArr[index] = *iter;
        prevPixel = *iter;
    }

    // write end marker
    encodedData.insert(encodedData.end(), QOI_END_MARKER.cbegin(), QOI_END_MARKER.cend());

    return encodedData;
}

void writeToPPMFile(const std::filesystem::path &filename, const DecodedOutput &decodedData) {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n"
        << decodedData.d_header.d_width << " " << decodedData.d_header.d_height << "\n255\n";

    for (const auto &px : decodedData.d_pixels) {
        out.put(px.d_red);
        out.put(px.d_green);
        out.put(px.d_blue);
    }
}

void writeToQOIFile(const std::filesystem::path &filename, const std::vector<Byte> &encodedBytes) {
    std::ofstream out(filename, std::ios::binary);

    for (const auto &byte : encodedBytes) {
        out.put(byte);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <operation | encode or decode> <input file> \n";
    }

    std::string op = argv[1];
    std::string inputFile = argv[2];

    try {
        if (op == DECODE_OP) {
            const std::vector<Byte> buffer = readQOIFile(inputFile);
            std::cout << "QOI size: " << buffer.size() << std::endl;
            auto outBuffer = decodeQOI(buffer);
            writeToPPMFile("dummy.ppm", outBuffer);
        } else if (op == ENCODE_OP) {
            const PPMOutput ppmData = readPPMFile(inputFile);
            std::cout << "PPM size: " << ppmData.d_pixels.size() << std::endl;
            const std::vector<Byte> encodedBytes = encodeToQOI(ppmData);
            writeToQOIFile("dummy.qoi", encodedBytes);
        } else {
            throw std::runtime_error("Invalid operation selected. Use either <encode> for qoi "
                                     "encoding or <decode> for qoi decoding.");
        }
    } catch (const std::exception &e) {
        std::cerr << "Error occurred: " << e.what() << '\n';
    }
}