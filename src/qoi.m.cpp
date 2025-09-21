#include <iostream>
#include <stdexcept>
#include <string>

#include <qoi_constants.h>
#include <qoi_decoder.h>
#include <qoi_encoder.h>
#include <qoi_types.h>
#include <qoi_utils.h>

using namespace qoi;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <operation | encode or decode> <input file> <output file> \n";
    }

    const std::string op = argv[1];
    const std::string inputFile = argv[2];
    const std::string outputFile = argv[3];

    try {
        if (op == DECODE_OP) {
            auto decoder = Decoder(0);
            const FileOutput output = readQOIFile(inputFile);
            const DecodedOutput outBuffer = decoder.decodeQOI(output);
            writeToPPMFile(outputFile, outBuffer);
        } else if (op == ENCODE_OP) {
            auto encoder = Encoder();
            const FileOutput ppmData = readPPMFile(inputFile);
            const EncodedOutput encodedBytes = encoder.encodeToQOI(ppmData);
            writeToQOIFile(outputFile, encodedBytes);
        } else {
            throw std::runtime_error("Invalid operation selected. Use either <encode> for qoi "
                                     "encoding or <decode> for qoi decoding.");
        }
    } catch (const std::exception &e) {
        std::cerr << "Error occurred: " << e.what() << '\n';
    }
}
