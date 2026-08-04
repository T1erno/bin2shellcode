#include "converter/formatter.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace b2s {

namespace {

std::string byteToHex(unsigned char b) {
  std::ostringstream ss;
  ss << std::hex << std::setw(2) << std::setfill('0') << int(b);
  return ss.str();
}

template <typename EmitByte, typename EmitBreak>
void forEachLine(const std::vector<unsigned char> &data, int perLine,
                 EmitByte emitByte, EmitBreak emitBreak) {
  int count = 0;
  for (size_t i = 0; i < data.size(); ++i) {
    emitByte(data[i], i);
    if (++count == perLine && i + 1 < data.size()) {
      emitBreak();
      count = 0;
    }
  }
}

} // namespace

std::string Formatter::formatOutput(const EncodedResult &encoded, Format format,
                                    const std::string &varName) {
  std::ostringstream out;

  // 1. Base64 cipher formatting for all language templates
  if (encoded.type == CipherType::Base64) {
    std::string b64Str = encoded.rawString;
    if (b64Str.empty()) {
      b64Str = Base64Encoder().encode(encoded.bytes).rawString;
    }
    switch (format) {
    case Format::Raw:
      out << b64Str << "\n";
      break;
    case Format::CString:
      out << "const char* " << varName << " = \"" << b64Str << "\";\n";
      break;
    case Format::Ruby:
      out << varName << " = \"" << b64Str << "\"\n";
      break;
    case Format::Python:
      out << varName << " = \"" << b64Str << "\"\n";
      break;
    case Format::PowerShell:
      out << "$" << varName << " = \"" << b64Str << "\";\n";
      break;
    case Format::CSharp:
      out << "string " << varName << " = \"" << b64Str << "\";\n";
      break;
    case Format::Go:
      out << "var " << varName << " = \"" << b64Str << "\"\n";
      break;
    case Format::Rust:
      out << "let " << varName << ": &str = \"" << b64Str << "\";\n";
      break;
    }
    return out.str();
  }

  // 2. Hex continuous string cipher formatting
  if (encoded.type == CipherType::Hex) {
    std::string hexStr = encoded.rawString;
    if (hexStr.empty()) {
      hexStr = HexEncoder().encode(encoded.bytes).rawString;
    }
    switch (format) {
    case Format::Raw:
      out << hexStr << "\n";
      break;
    case Format::CString:
      out << "const char* " << varName << " = \"" << hexStr << "\";\n";
      break;
    case Format::Ruby:
      out << varName << " = \"" << hexStr << "\"\n";
      break;
    case Format::Python:
      out << varName << " = \"" << hexStr << "\"\n";
      break;
    case Format::PowerShell:
      out << "$" << varName << " = \"" << hexStr << "\";\n";
      break;
    case Format::CSharp:
      out << "string " << varName << " = \"" << hexStr << "\";\n";
      break;
    case Format::Go:
      out << "var " << varName << " = \"" << hexStr << "\"\n";
      break;
    case Format::Rust:
      out << "let " << varName << ": &str = \"" << hexStr << "\";\n";
      break;
    }
    return out.str();
  }

  // 3. Token list ciphers (IPv4, MAC, UUID)
  bool isTokenList =
      (encoded.type == CipherType::IPv4 || encoded.type == CipherType::MAC ||
       encoded.type == CipherType::UUID);

  if (isTokenList) {
    switch (format) {
    case Format::Raw: {
      for (const auto &tok : encoded.tokens) {
        out << tok << "\n";
      }
      break;
    }
    case Format::CString: {
      out << "const char* " << varName << "[] = {\n";
      for (size_t i = 0; i < encoded.tokens.size(); ++i) {
        out << "    \"" << encoded.tokens[i] << "\"";
        if (i + 1 < encoded.tokens.size())
          out << ",";
        out << "\n";
      }
      out << "};\n";
      break;
    }
    case Format::Ruby: {
      out << varName << " = [\n";
      for (size_t i = 0; i < encoded.tokens.size(); ++i) {
        out << "  \"" << encoded.tokens[i] << "\"";
        if (i + 1 < encoded.tokens.size())
          out << ",";
        out << "\n";
      }
      out << "]\n";
      break;
    }
    case Format::Python: {
      out << varName << " = [\n";
      for (size_t i = 0; i < encoded.tokens.size(); ++i) {
        out << "    \"" << encoded.tokens[i] << "\"";
        if (i + 1 < encoded.tokens.size())
          out << ",";
        out << "\n";
      }
      out << "]\n";
      break;
    }
    case Format::PowerShell: {
      out << "$" << varName << " = @(\n";
      for (size_t i = 0; i < encoded.tokens.size(); ++i) {
        out << "    \"" << encoded.tokens[i] << "\"";
        if (i + 1 < encoded.tokens.size())
          out << ",";
        out << "\n";
      }
      out << ");\n";
      break;
    }
    case Format::CSharp: {
      out << "string[] " << varName << " = new string[] {\n";
      for (size_t i = 0; i < encoded.tokens.size(); ++i) {
        out << "    \"" << encoded.tokens[i] << "\"";
        if (i + 1 < encoded.tokens.size())
          out << ",";
        out << "\n";
      }
      out << "};\n";
      break;
    }
    case Format::Go: {
      out << "var " << varName << " = []string{\n";
      for (const auto &tok : encoded.tokens) {
        out << "    \"" << tok << "\",\n";
      }
      out << "}\n";
      break;
    }
    case Format::Rust: {
      out << "let " << varName << ": &[&str] = &[\n";
      for (const auto &tok : encoded.tokens) {
        out << "    \"" << tok << "\",\n";
      }
      out << "];\n";
      break;
    }
    }
    return out.str();
  }

  // 4. Standard raw bytes formatting
  const auto &bytes = encoded.bytes;
  out << std::hex << std::setfill('0');

  switch (format) {
  case Format::Raw: {
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t idx) {
          out << std::setw(2) << int(b);
          if (idx + 1 < bytes.size() && (idx + 1) % PER_LINE != 0) {
            out << ' ';
          }
        },
        [&]() { out << '\n'; });
    out << '\n';
    break;
  }
  case Format::CString: {
    out << "unsigned char " << varName << "[] =\n\"";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t) { out << "\\x" << byteToHex(b); },
        [&]() { out << "\"\n\""; });
    out << "\";\n";
    break;
  }
  case Format::Ruby: {
    out << "unsigned char " << varName << "[] =\n\"";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t) { out << "\\x" << byteToHex(b); },
        [&]() { out << "\"\n\""; });
    out << "\"\n";
    break;
  }
  case Format::Python: {
    out << varName << " = \"\"\n";
    out << varName << " = b\"";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t) { out << "\\x" << byteToHex(b); },
        [&]() { out << "\"\n"
                    << varName << " = b\""; });
    out << "\"\n";
    break;
  }
  case Format::PowerShell: {
    out << "[Byte[]] $" << varName << " = (";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t idx) {
          out << "0x" << std::setw(2) << int(b);
          if (idx + 1 < bytes.size()) {
            out << ",";
          }
        },
        [&]() { out << '\n'; });
    out << ");\n";
    break;
  }
  case Format::CSharp: {
    out << "byte[] " << varName << " = new byte[] {\n";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t idx) {
          out << "0x" << std::setw(2) << int(b);
          if (idx + 1 < bytes.size()) {
            out << ",";
          }
        },
        [&]() { out << '\n'; });
    out << "};\n";
    break;
  }
  case Format::Go: {
    out << "var " << varName << " = []byte{\n";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t) {
          out << "0x" << std::setw(2) << int(b) << ",";
        },
        [&]() { out << '\n'; });
    out << "}\n";
    break;
  }
  case Format::Rust: {
    out << "let " << varName << ": &[u8] = &[\n";
    forEachLine(
        bytes, PER_LINE,
        [&](unsigned char b, size_t) {
          out << "0x" << std::setw(2) << int(b) << ",";
        },
        [&]() { out << '\n'; });
    out << "];\n";
    break;
  }
  }

  return out.str();
}

} // namespace b2s
