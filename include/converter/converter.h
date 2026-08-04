#ifndef CONVERTER_H
#define CONVERTER_H

#include "converter/encoder.h"
#include <string>
#include <vector>

namespace b2s {

/**
 * Output format identifiers (Programming Languages & Templates).
 */
enum class Format {
  Raw = 1,
  CString,
  Ruby,
  Python,
  PowerShell,
  CSharp,
  Go,
  Rust
};

/** Number of elements printed per line when formatting. */
constexpr int PER_LINE = 15;

std::vector<unsigned char> readFile(const std::string &path);

/**
 * Convert a binary buffer into a textual representation according to
 * `format` and `cipher`. Sequence of operations:
 * b2s -> Encoding (Cipher) -> Output Formatting
 */
std::string convert(const std::vector<unsigned char> &bytes, Format format,
                    const std::string &varName,
                    CipherType cipher = CipherType::Raw);

} // namespace b2s

#endif // CONVERTER_H
