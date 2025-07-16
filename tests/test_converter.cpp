#include "converter/converter.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

#define FORMAT_LIST                                                            \
  X(Raw, "raw")                                                                \
  X(Base64, "b64")                                                             \
  X(C, "c")                                                                    \
  X(Ruby, "rb")                                                                \
  X(Python, "py")                                                              \
  X(PowerShell, "ps1")                                                         \
  X(CSharp, "cs")                                                              \
  X(Hex, "hex")                                                                \
  X(UUID, "uuid")                                                              \
  X(Go, "go")                                                                  \
  X(Rust, "rs")

enum class Format {
#define X(name, ext) name,
  FORMAT_LIST
#undef X
};

enum {
  FormatCount =
#define X(name, ext) +1
      0 FORMAT_LIST
#undef X
};

static constexpr std::array<std::string_view, FormatCount> FormatExtensions = {
#define X(name, ext) ext,
    FORMAT_LIST
#undef X
};

std::string readFileText(const fs::path &filePath) {
  std::ifstream input{filePath, std::ios::binary};
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string_view getExtension(Format fmt) noexcept {
  return FormatExtensions.at(static_cast<size_t>(fmt));
}

void verifyFormat(const byte_buffer &buffer, Format fmt) {
  fs::path expectedFile =
      fs::path{TestDataDir} / ("shellcode." + std::string(getExtension(fmt)));
  std::string expected = readFileText(expectedFile);
  std::string varName =
      (fmt == Format::Ruby ? "buf" : "shellcode"); // Specific test for Ruby
  // "b2s::convert" expects the format numbers starting from 1 while "Format"
  // enum is zero based
  std::string output = b2s::convert(buffer, static_cast<int>(fmt) + 1, varName);
  assert(output == expected);
}

int main(int argc, char *argv[]) {
  fs::path binaryFile = fs::path{TestDataDir} / "MessageBox.bin";
  byte_buffer buffer = b2s::readFile(binaryFile.string());
  assert(buffer.size() == 299);

  if (argc > 1) {
    int idx = std::atoi(argv[1]);
    verifyFormat(buffer, static_cast<Format>(idx - 1));
    return EXIT_SUCCESS;
  }

  for (size_t i = 0; i < FormatCount; ++i) {
    verifyFormat(buffer, static_cast<Format>(i));
  }

  return EXIT_SUCCESS;
}
