#include "converter/converter.h"
#include "converter/encoder.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
using byte_buffer = std::vector<unsigned char>;

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "tests"
#endif
static constexpr std::string_view TestDataDir{TEST_DATA_DIR};

struct FormatInfo {
  b2s::Format fmt;
  std::string_view ext;
};

static constexpr std::array<FormatInfo, 8> Formats = {
    FormatInfo{b2s::Format::Raw, "raw"},
    FormatInfo{b2s::Format::CString, "c"},
    FormatInfo{b2s::Format::Ruby, "rb"},
    FormatInfo{b2s::Format::Python, "py"},
    FormatInfo{b2s::Format::PowerShell, "ps1"},
    FormatInfo{b2s::Format::CSharp, "cs"},
    FormatInfo{b2s::Format::Go, "go"},
    FormatInfo{b2s::Format::Rust, "rs"}};

struct EncodingInfo {
  b2s::CipherType cipher;
  std::string_view ext;
};

static constexpr std::array<EncodingInfo, 5> Encodings = {
    EncodingInfo{b2s::CipherType::Base64, "b64"},
    EncodingInfo{b2s::CipherType::Hex, "hex"},
    EncodingInfo{b2s::CipherType::UUID, "uuid"},
    EncodingInfo{b2s::CipherType::IPv4, "ipv4"},
    EncodingInfo{b2s::CipherType::MAC, "mac"}};

std::string readFileText(const fs::path &filePath) {
  std::ifstream input{filePath, std::ios::binary};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string_view getExtension(b2s::Format fmt) {
  for (const auto &info : Formats) {
    if (info.fmt == fmt) {
      return info.ext;
    }
  }
  return {};
}

void verifyFormat(const byte_buffer &buffer, b2s::Format fmt) {
  fs::path expectedFile =
      fs::path{TestDataDir} / ("shellcode." + std::string(getExtension(fmt)));
  std::string expected = readFileText(expectedFile);
  std::string varName = (fmt == b2s::Format::Ruby ? "buf" : "shellcode");
  std::string output = b2s::convert(buffer, fmt, varName, b2s::CipherType::Raw);
  assert(output == expected);
}

void verifyEncoding(const byte_buffer &buffer, const EncodingInfo &encInfo) {
  fs::path expectedFile =
      fs::path{TestDataDir} / ("shellcode." + std::string(encInfo.ext));
  std::string expected = readFileText(expectedFile);
  std::string output =
      b2s::convert(buffer, b2s::Format::Raw, "shellcode", encInfo.cipher);
  assert(output == expected);
}

int main(int argc, char *argv[]) {
  fs::path binaryFile = fs::path{TestDataDir} / "MessageBox.bin";
  byte_buffer buffer = b2s::readFile(binaryFile.string());
  assert(buffer.size() == 299);

  if (argc > 1) {
    int idx = std::atoi(argv[1]);
    if (idx >= 1 && idx <= static_cast<int>(Formats.size())) {
      verifyFormat(buffer, Formats[idx - 1].fmt);
    } else if (idx > static_cast<int>(Formats.size()) &&
               idx <= static_cast<int>(Formats.size() + Encodings.size())) {
      verifyEncoding(buffer, Encodings[idx - 1 - Formats.size()]);
    }
    return EXIT_SUCCESS;
  }

  for (const auto &info : Formats) {
    verifyFormat(buffer, info.fmt);
  }

  for (const auto &encInfo : Encodings) {
    verifyEncoding(buffer, encInfo);
  }

  return EXIT_SUCCESS;
}
