#pragma once

#include <qoi_constants.h>
#include <qoi_types.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

namespace qoi {
inline auto byte_ptr(char *p) -> Byte * { return reinterpret_cast<Byte *>(p); }

inline auto char_ptr(Byte *p) -> char * { return reinterpret_cast<char *>(p); }

inline auto hashIndex(const Pixel &pixel) -> Hash {
    return ((pixel.d_red * 3) + (pixel.d_green * 5) + (pixel.d_blue * 7) + (pixel.d_alpha * 11)) %
           64;
}

inline auto printByte(Byte byte) -> void {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ' ';
}

inline auto printBuffer(const std::vector<Byte> &buffer) -> void {
    for (auto &byte : buffer) {
        printByte(byte);
    }

    std::cout << std::dec << std::endl; // reset stream to decimal
}

inline auto printMagicTag(const std::array<Byte, 4> &magicTag) -> void {
    std::cout << "Magic tag: ";
    for (auto &ch : magicTag) {
        std::cout << ch;
    }

    std::cout << std::endl;
}

inline auto printQOIHeader(const QOIHeader &header) -> void {
    printMagicTag(header.d_magic);

    std::cout << "Width: " << header.d_width << "\n"
              << "Height: " << header.d_height << "\n"
              << "Channels: " << static_cast<unsigned int>(header.d_channels) << "\n"
              << "Colorspace: " << static_cast<unsigned int>(header.d_colorspace) << "\n";
}

inline auto printPixel(const Pixel &pixel) -> void {
    std::cout << "RED: " << static_cast<uint32_t>(pixel.d_red)
              << ", GREEN: " << static_cast<uint32_t>(pixel.d_green)
              << ", BLUE: " << static_cast<uint32_t>(pixel.d_blue)
              << ", ALPHA: " << static_cast<uint32_t>(pixel.d_alpha) << std::endl;
}

inline auto readU32(const std::vector<Byte> &buffer, std::size_t &offset) -> std::uint32_t {
    uint32_t red = buffer[offset++] << 24;
    uint32_t blue = buffer[offset++] << 16;
    uint32_t green = buffer[offset++] << 8;
    uint32_t alpha = buffer[offset++];
    return (red | blue | green | alpha);
}

inline auto writeU32(std::uint32_t value, std::vector<Byte> &encodedBuffer) -> void {
    encodedBuffer.emplace_back((value >> 24) & 0xFF);
    encodedBuffer.emplace_back((value >> 16) & 0xFF);
    encodedBuffer.emplace_back((value >> 8) & 0xFF);
    encodedBuffer.emplace_back(value & 0xFF);
}

inline auto convertBytesToPixel(const std::vector<Byte> &bytes, std::vector<Pixel> &pixels)
    -> void {
    for (int iter = 0; iter < pixels.size(); ++iter) {
        if ((iter * 3 + 2) >= bytes.size()) {
            throw std::runtime_error("The data is corrupted or incomplete");
        }
        pixels[iter] = {.d_red = bytes[iter * 3],
                        .d_green = bytes[iter * 3 + 1],
                        .d_blue = bytes[iter * 3 + 2],
                        .d_alpha = 255};
    }
}

inline auto extractHeader(const std::vector<Byte> &buffer, QOIHeader &header, std::size_t &offset)
    -> void {
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

inline auto hasValidEndMarker(const std::vector<Byte> &buffer) -> bool {
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

constexpr FileOutput readQOIFile(const std::filesystem::path &filename) {
    std::ifstream file{filename, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename.string());
    }

    std::vector<Byte> buffer((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    if (buffer.size() > MAX_FILE_SIZE) {
        throw std::runtime_error(filename.string() + " exceeds the limit of 1GB");
    }

    if (buffer.size() < QOI_HEADER_SIZE) {
        throw std::runtime_error("QOI file header is missing!");
    }

    std::size_t offset = 0;
    QOIHeader header{};
    extractHeader(buffer, header, offset);

    if (!std::equal(header.d_magic.begin(), header.d_magic.end(), QOI_MAGIC_TAG.begin())) {
        throw std::runtime_error("Unsupported File format. Expect QOI file.");
    }

    if (!hasValidEndMarker(buffer)) {
        throw std::runtime_error("QOI file is corrupted or incomplete");
    }

    return {.d_width = header.d_width,
            .d_height = header.d_height,
            .d_channels = header.d_channels,
            .d_colorspace = header.d_colorspace,
            .d_bytes = std::vector<Byte>(buffer.begin() + offset, buffer.end())};
}

constexpr FileOutput readPPMFile(const std::filesystem::path &filename) {
    std::ifstream file{filename, std::ios::binary};
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename.string());
    }

    std::string tag;
    file >> tag;
    if (tag != PPM_MAGIC_TAG) {
        throw std::runtime_error(
            tag + " is an unsupported PPM format. Only P6 (binary RGB) is supported.");
    }

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

    if (maxPixelValue != PPM_MAX_PIXEL_VALUE) {
        throw std::runtime_error("Only 8-bit PPM files (max color 255) are supported.");
    }

    file.ignore();

    std::vector<Byte> bytes(width * height * 3);

    file.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
    if (!file) {
        throw std::runtime_error("Unable to read pixel data from PPM file");
    }

    return {
        .d_width = width, .d_height = height, .d_channels = 3, .d_colorspace = 0, .d_bytes = bytes};
}

inline auto writeToPPMFile(const std::filesystem::path &filename, const DecodedOutput &decodedData)
    -> void {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << decodedData.d_width << " " << decodedData.d_height << "\n255\n";

    for (const auto &px : decodedData.d_pixels) {
        out.put(px.d_red);
        out.put(px.d_green);
        out.put(px.d_blue);
    }
}

inline auto writeToQOIFile(const std::filesystem::path &filename, const EncodedOutput &encodedData)
    -> void {
    std::ofstream out(filename, std::ios::binary);

    for (const auto &byte : encodedData.d_bytes) {
        out.put(byte);
    }
}

} // namespace qoi
