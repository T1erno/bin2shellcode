#include "converter/converter.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace b2s {

static const char *B64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/";

static std::string encodeBase64(const std::vector<unsigned char> &in) {
  std::string out;
  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(B64_CHARS[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) {
    out.push_back(B64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
  }
  while (out.size() % 4) {
    out.push_back('=');
  }
  return out;
}

std::vector<unsigned char> readFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  std::istreambuf_iterator<char> it_begin(file), it_end;
  return std::vector<unsigned char>(it_begin, it_end);
}

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

std::string convert(const std::vector<unsigned char> &bytes, Format format,
                    const std::string &varName) {
  std::ostringstream out;
  out << std::hex << std::setfill('0');

  switch (format) {
  case Format::Raw: {
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t idx) {
                  out << std::setw(2) << int(b);
                  if (idx + 1 < bytes.size() &&
                      (idx + 1) % PER_LINE != 0) {
                    out << ' ';
                  }
                },
                [&]() { out << '\n'; });
    out << '\n';
    break;
  }
  case Format::Base64: { // Base64 string only
    out << encodeBase64(bytes) << '\n';
    break;
  }
  case Format::CString: { // C string literal with escapes
    out << "unsigned char " << varName << "[] =\n\"";
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t) {
                  out << "\\x" << byteToHex(b);
                },
                [&]() { out << "\"\n\""; });
    out << "\";\n";
    break;
  }
  case Format::Ruby: { // Ruby (as C escaped string without semicolon)
    out << "unsigned char " << varName << "[] =\n\"";
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t) {
                  out << "\\x" << byteToHex(b);
                },
                [&]() { out << "\"\n\""; });
    out << "\"\n";
    break;
  }
  case Format::Python: {
    out << varName << " = \"\"\n";
    out << varName << " = b\"";
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t) {
                  out << "\\x" << byteToHex(b);
                },
                [&]() { out << "\"\n" << varName << " = b\""; });
    out << "\"\n";
    break;
  }
  case Format::PowerShell: {
    out << "[Byte[]] $" << varName << " = (";
    forEachLine(bytes, PER_LINE,
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
    forEachLine(bytes, PER_LINE,
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
  case Format::HexString: {
    for (unsigned char b : bytes) {
      out << std::setw(2) << int(b);
    }
    out << '\n';
    break;
  }
  case Format::UUID: {
    std::vector<unsigned char> tmp = bytes;
    size_t pad = (16 - tmp.size() % 16) % 16;
    tmp.insert(tmp.end(), pad, 0);
    for (size_t off = 0; off < tmp.size(); off += 16) {
      std::ostringstream uuid;
      auto hex2 = [&](int i) {
        std::ostringstream s;
        s << std::hex << std::setw(2) << std::setfill('0') << int(tmp[off + i]);
        return s.str();
      };
      auto hex4 = [&](int i) {
        std::ostringstream s;
        int v = (tmp[off + i] << 8) | tmp[off + i + 1];
        s << std::hex << std::setw(4) << std::setfill('0') << v;
        return s.str();
      };
      uuid << hex4(0) << hex4(2) << '-' << hex2(4) << hex2(5) << '-' << hex2(6)
           << hex2(7) << '-' << hex2(8) << hex2(9) << '-' << hex4(10)
           << hex4(12);
      out << uuid.str() << '\n';
    }
    break;
  }
  case Format::Go: {
    out << "var " << varName << " = []byte{\n";
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t) {
                  out << "0x" << std::setw(2) << int(b) << ",";
                },
                [&]() { out << '\n'; });
    out << "}\n";
    break;
  }
  case Format::Rust: {
    out << "let " << varName << ": &[u8] = &[\n";
    forEachLine(bytes, PER_LINE,
                [&](unsigned char b, size_t) {
                  out << "0x" << std::setw(2) << int(b) << ",";
                },
                [&]() { out << '\n'; });
    out << "];\n";
    break;
  }
  default:
    throw std::runtime_error("Unknown format");
  }
  return out.str();
}

}
