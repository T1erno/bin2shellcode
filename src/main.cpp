#include "converter/converter.h"
#include "converter/encoder.h"
#include <cstring>
#include <iostream>
#include <string>

static void printHelp(const char *prog) {
  std::cout
      // clang-format off
        << "bin2shellcode " << B2S_VERSION << "\n"
        << "=======================\n\n"
        << "Usage:\n"
        << "  bin2shellcode --input <path> [--cipher <type>] [--format <n>] [--varname <name>]\n"
        << "  bin2shellcode <path> <n> [<name>]\n\n"
        << "Options:\n"
        << "  -i, --input <path>      Path to the .bin file to convert (required)\n"
        << "  -c, --cipher <type>     Encoding/Cipher (raw, base64, hex, uuid, ipv4, mac) (default: raw)\n"
        << "  -f, --format <n>        Programming language output format (1-8) (default: 1)\n"
        << "  -n, --varname <name>    Name of the output variable (default: shellcode)\n"
        << "  -h, --help              Show this help and exit\n"
        << "  -v, --version           Show version and exit\n\n"
        << "Formats (Programming Language Output):\n"
        << "  #   Language        Output Template\n"
        << "  -------------------------------------------------------\n"
        << "  1   Raw             Raw text / stdout\n"
        << "  2   C               unsigned char shellcode[] = \"\\x90\\x90...\"\n"
        << "  3   Ruby            unsigned char buf[] = \"\\x90\\x90...\"\n"
        << "  4   Python          shellcode = b\"\\x90\\x90...\"\n"
        << "  5   PowerShell      [Byte[]] $shellcode = 0x90,0x90,...\n"
        << "  6   C#              byte[] shellcode = { 0x90, 0x90, ... }\n"
        << "  7   Go              []byte{0x90, 0x90, ...}\n"
        << "  8   Rust            let shellcode: &[u8] = &[0x90, 0x90, ...];\n\n"
        << "Encodings / Ciphers (-c, --cipher):\n"
        << "  Type        Description\n"
        << "  -------------------------------------------------------\n"
        << "  raw         Raw bytes / no cipher (default)\n"
        << "  base64, b64 Base64 string encoding\n"
        << "  hex         Continuous hex string\n"
        << "  uuid        UUID format grouping (16 bytes per UUID)\n"
        << "  ipv4, ip    IPv4 address grouping (4 bytes per IP)\n"
        << "  mac         MAC address grouping (6 bytes per MAC)\n\n"
        << "Examples:\n"
        << "  Convert \"payload.bin\" to C byte array:\n"
        << "      bin2shellcode -i payload.bin -f 2\n"
        << "  Convert \"payload.bin\" using Base64 encoding in C format:\n"
        << "      bin2shellcode -i payload.bin -c base64 -f 2\n"
        << "  Convert \"payload.bin\" using IPv4 encoding in Python format:\n"
        << "      bin2shellcode -i payload.bin -c ipv4 -f 4\n";
  // clang-format on
}

static void printVersion() {
  std::cout << "bin2shellcode " << B2S_VERSION << '\n';
}

int main(int argc, char *argv[]) {
  std::string filePath;
  int formatValue = 1; // Default to Raw format
  bool formatSpecified = false;
  std::string varName;
  std::string cipherStr;
  bool cipherSpecified = false;

  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
      printHelp(argv[0]);
      return 0;
    } else if (std::strcmp(arg, "-v") == 0 ||
               std::strcmp(arg, "--version") == 0) {
      printVersion();
      return 0;
    } else if ((std::strcmp(arg, "--input") == 0 ||
                std::strcmp(arg, "-i") == 0) &&
               i + 1 < argc) {
      filePath = argv[++i];
    } else if ((std::strcmp(arg, "--format") == 0 ||
                std::strcmp(arg, "-f") == 0) &&
               i + 1 < argc) {
      try {
        formatValue = std::stoi(argv[++i]);
        formatSpecified = true;
      } catch (...) {
        std::cerr << "Invalid format: " << argv[i] << "\n";
        return 1;
      }
    } else if ((std::strcmp(arg, "--cipher") == 0 ||
                std::strcmp(arg, "-c") == 0 ||
                std::strcmp(arg, "--encoding") == 0) &&
               i + 1 < argc) {
      cipherStr = argv[++i];
      cipherSpecified = true;
    } else if ((std::strcmp(arg, "--varname") == 0 ||
                std::strcmp(arg, "-n") == 0) &&
               i + 1 < argc) {
      varName = argv[++i];
    } else if (filePath.empty()) {
      filePath = arg;
    } else if (!formatSpecified) {
      try {
        formatValue = std::stoi(arg);
        formatSpecified = true;
      } catch (...) {
        std::cerr << "Invalid format: " << arg << "\n";
        return 1;
      }
    } else if (varName.empty()) {
      varName = arg;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 1;
    }
  }

  if (filePath.empty()) {
    std::cerr << "Missing required input file.\n";
    printHelp(argv[0]);
    return 1;
  }

  if (formatValue < static_cast<int>(b2s::Format::Raw) ||
      formatValue > static_cast<int>(b2s::Format::Rust)) {
    std::cerr << "Invalid format index: " << formatValue
              << ". Use 1-8 for language formats.\n";
    return 1;
  }

  if (varName.empty()) {
    varName = "buf";
  }

  b2s::CipherType cipher = b2s::CipherType::Raw;
  if (cipherSpecified) {
    if (cipherStr == "raw")
      cipher = b2s::CipherType::Raw;
    else if (cipherStr == "base64" || cipherStr == "b64")
      cipher = b2s::CipherType::Base64;
    else if (cipherStr == "ipv4" || cipherStr == "ip")
      cipher = b2s::CipherType::IPv4;
    else if (cipherStr == "mac")
      cipher = b2s::CipherType::MAC;
    else if (cipherStr == "uuid")
      cipher = b2s::CipherType::UUID;
    else if (cipherStr == "hex")
      cipher = b2s::CipherType::Hex;
    else {
      std::cerr << "Unknown cipher/encoding: " << cipherStr << "\n";
      return 1;
    }
  }

  b2s::Format format = static_cast<b2s::Format>(formatValue);
  auto bytes = b2s::readFile(filePath);
  try {
    std::cout << b2s::convert(bytes, format, varName, cipher);
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
  return 0;
}
