#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>
#include <vector>

namespace b2s {

/**
 * Output format identifiers.
 */
enum class Format {
  Raw = 1,
  Base64,
  CString,
  Ruby,
  Python,
  PowerShell,
  CSharp,
  HexString,
  UUID,
  Go,
  Rust
};

/** Number of elements printed per line when formatting. */
constexpr int PER_LINE = 15;

std::vector<unsigned char> readFile(const std::string &path);

/**
 * Convert a binary buffer into a textual representation according to
 * `format`.  The `varName` parameter is used when the output requires a
 * variable declaration (for example C arrays).
 */
std::string convert(const std::vector<unsigned char> &bytes, Format format,
                    const std::string &varName);

}

#endif
