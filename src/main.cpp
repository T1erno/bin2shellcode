#include <iostream>
#include <cstring>
#include <string>
#include "converter/converter.h"

static void printHelp(const char* prog) {
    std::cout
        << "bin2shellcode " << B2S_VERSION << "\n"
        << "=======================\n\n"
        << "Usage:\n"
        << "  bin2shellcode --input <path> --format <n> [--varname <name>]\n"
        << "  bin2shellcode <path> <n> [<name>]\n\n"
        << "Options:\n"
        << "  -i, --input <path>      Path to the binary to convert (required)\n"
        << "  -f, --format <n>        Output format (1-11) (required)\n"
        << "  -n, --varname <name>    Name of the output variable (default: shellcode)\n"
        << "  -h, --help              Show this help and exit\n"
        << "  -v, --version           Show version and exit\n\n"
        << "Formats:\n"
        << "  #   Description\n"
        << "  -------------------------------------------------------\n"
        << "  1   Raw hex bytes                   (fc 48 81 ...)\n"
        << "  2   Base64 string                   (SGVsbG8uLi4=)\n"
        << "  3   C                               (unsigned char shellcode[] = \"\\x90\\x90...\")\n"
        << "  4   Ruby                            (unsigned char buf[] = \"\\x90\\x90...\")\n"
        << "  5   Python                          (shellcode = b\"\\x90\\x90...\")\n"
        << "  6   PowerShell                      ([Byte[]] $shellcode = 0x90,0x90,...)\n"
        << "  7   C#                              (byte[] shellcode = { 0x90, 0x90, ... })\n"
        << "  8   Hex string                      (9090...)\n"
        << "  9   UUID list                       (fc4881e4-f0ff-ffff-...)\n"
        << "  10  Go                              ([]byte{0x90, 0x90, ...})\n"
        << "  11  Rust                            (let shellcode: &[u8] = &[0x90, 0x90, ...];)\n\n"
        << "Examples:\n"
        << "  Convert \"payload.bin\" to a C byte array:\n"
        << "      bin2shellcode -i payload.bin -f 1\n";
}

static void printVersion() {
    std::cout << "bin2shellcode " << B2S_VERSION << '\n';
}

int main(int argc, char* argv[]) {
    std::string filePath;
    int fmt = 0;
    std::string varName;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
            printHelp(argv[0]);
            return 0;
        } else if (std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--version") == 0) {
            printVersion();
            return 0;
        } else if ((std::strcmp(arg, "--input") == 0 ||
                    std::strcmp(arg, "-i") == 0) && i + 1 < argc) {
            filePath = argv[++i];
        } else if ((std::strcmp(arg, "--format") == 0 ||
                    std::strcmp(arg, "-f") == 0) && i + 1 < argc) {
            fmt = std::stoi(argv[++i]);
        } else if ((std::strcmp(arg, "--varname") == 0 ||
                    std::strcmp(arg, "-n") == 0) && i + 1 < argc) {
            varName = argv[++i];
        } else if (filePath.empty()) {
            filePath = arg;
        } else if (fmt == 0) {
            fmt = std::stoi(arg);
        } else if (varName.empty()) {
            varName = arg;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (filePath.empty() || fmt == 0) {
        std::cerr << "Not enough arguments.\n";
        printHelp(argv[0]);
        return 1;
    }

    if (fmt < 1 || fmt > 11) {
        std::cerr << "Invalid format. Use -h for help.\n";
        return 1;
    }

    if (varName.empty()) {
        varName = "buf";
    }

    auto buf = b2s::readFile(filePath);
    try {
        std::cout << b2s::convert(buf, fmt, varName);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
