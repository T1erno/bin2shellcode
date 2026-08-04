#ifndef FORMATTER_H
#define FORMATTER_H

#include "converter/converter.h"
#include "converter/encoder.h"
#include <string>

namespace b2s {

/**
 * Formats encoded data (EncodedResult) into target output templates.
 */
class Formatter {
public:
  static std::string formatOutput(const EncodedResult &encoded,
                                  Format format,
                                  const std::string &varName);
};

} // namespace b2s

#endif // FORMATTER_H
