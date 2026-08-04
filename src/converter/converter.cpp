#include "converter/converter.h"
#include "converter/encoder.h"
#include "converter/formatter.h"

#include <fstream>
#include <iterator>

namespace b2s {

std::vector<unsigned char> readFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  std::istreambuf_iterator<char> it_begin(file), it_end;
  return std::vector<unsigned char>(it_begin, it_end);
}

std::string convert(const std::vector<unsigned char> &bytes, Format format,
                    const std::string &varName, CipherType cipher) {
  // Step 1: Encoding / Cipher stage
  auto encoder = createEncoder(cipher);
  EncodedResult encoded = encoder->encode(bytes);

  // Step 2: Output formatting stage
  return Formatter::formatOutput(encoded, format, varName);
}

} // namespace b2s
