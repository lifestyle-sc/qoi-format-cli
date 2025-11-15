#define REQUIRED_ARGS                                                                              \
    REQUIRED_STRING_ARG(                                                                           \
        operation, "operation",                                                                    \
        "Operation to perform. Use <encode> to encode to qoi and <decode> to decode from qoi")     \
    REQUIRED_STRING_ARG(inputFile, "input", "Input file path")                                     \
    REQUIRED_STRING_ARG(outputFile, "output", "Output file path")

#define OPTIONAL_ARGS                                                                              \
    OPTIONAL_ARG(char const *, fileFormat, "ppm", "-f", "fileFormat",                              \
                 "Image format to encode from or decode to. Default is set to <ppm>, but "         \
                 "also supports <png>",                                                            \
                 "%s", )

#define BOOLEAN_ARGS BOOLEAN_ARG(help, "-h", "Show help")

#include <iostream>
#include <stdexcept>

#include <qoi_constants.h>
#include <qoi_decoder.h>
#include <qoi_encoder.h>
#include <qoi_types.h>
#include <qoi_utils.h>

#include <easyargs.h>

using namespace qoi;

int main(int argc, char *argv[]) {
    args_t args = make_default_args();

    if (!parse_args(argc, argv, &args) || args.help) {
        print_help(argv[0]);
        return 1;
    }

    try {
        if (args.operation == DECODE_OP) {
            auto decoder = Decoder(0);
            if (args.fileFormat == PPM_FILE_FORMAT) {
                const FileOutput output = readQOIFile(args.inputFile);
                const DecodedOutput outBuffer = decoder.decodeQOI(output);
                writeToPPMFile(args.outputFile, outBuffer);
            } else if (args.fileFormat == PNG_FILE_FORMAT) {
                const FileOutput output = readQOIFile(args.inputFile);
                const DecodedOutput outBuffer = decoder.decodeQOI(output);
                writeToPNGFile(args.outputFile, outBuffer);
            } else {
                throw std::runtime_error("Invalid file format selected. Supported file format "
                                         "include: <ppm> and <png>.");
            }
        } else if (args.operation == ENCODE_OP) {
            auto encoder = Encoder();
            if (args.fileFormat == PPM_FILE_FORMAT) {
                const FileOutput ppmData = readPPMFile(args.inputFile);
                const EncodedOutput encodedBytes = encoder.encodeToQOI(ppmData);
                writeToQOIFile(args.outputFile, encodedBytes);
            } else if (args.fileFormat == PNG_FILE_FORMAT) {
                const FileOutput pngData = readPNGFile(args.inputFile);
                const EncodedOutput encodedBytes = encoder.encodeToQOI(pngData);
                writeToQOIFile(args.outputFile, encodedBytes);
            } else {
                throw std::runtime_error("Invalid file format selected. Supported file format "
                                         "include: <ppm> and <png>.");
            }
        } else {
            throw std::runtime_error("Invalid operation selected. Use either <encode> for qoi "
                                     "encoding or <decode> for qoi decoding.");
        }
    } catch (const std::exception &e) {
        std::cerr << "Error occurred: " << e.what() << '\n';
    }
}
