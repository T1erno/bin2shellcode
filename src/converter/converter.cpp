#include "converter/converter.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace b2s {

static const char* B64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static std::string encodeBase64(const std::vector<unsigned char>& in) {
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

std::vector<unsigned char> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    std::istreambuf_iterator<char> it_begin(file), it_end;
    return std::vector<unsigned char>(it_begin, it_end);
}

std::string convert(const std::vector<unsigned char>& buf, int fmt,
                    const std::string& varName) {
    const int perLine = 15;
    int ctr = 1;
    std::ostringstream out;

    switch (fmt) {
        case 1: { // Raw hex bytes
            for (size_t i = 0; i < buf.size(); ++i) {
                std::ostringstream ss;
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << int(buf[i]);
                out << ss.str();
                if (i + 1 < buf.size()) {
                    if (ctr++ == perLine) {
                        out << "\n";
                        ctr = 1;
                    } else {
                        out << ' ';
                    }
                }
            }
            out << "\n";
            break;
        }
        case 2: { // Base64 string only
            out << encodeBase64(buf) << "\n";
            break;
        }
        case 3: { // C string literal with escapes
            out << "unsigned char " << varName << "[] =\n\"";
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str();
                if (ctr++ == perLine) {
                    out << "\"\n\"";
                    ctr = 1;
                }
            }
            out << "\";\n";
            break;
        }
        case 4: { // Ruby (as C escaped string without semicolon)
            out << "unsigned char " << varName << "[] =\n\"";
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str();
                if (ctr++ == perLine) {
                    out << "\"\n\"";
                    ctr = 1;
                }
            }
            out << "\"\n";
            break;
        }
        case 5: { // Python
            out << varName << " = \"\"\n";
            out << varName << " = b\"";
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str();
                if (ctr++ == perLine) {
                    out << "\"\n" << varName << " = b\"";
                    ctr = 1;
                }
            }
            out << "\"\n";
            break;
        }
        case 6: { // PowerShell
            out << "[Byte[]] $" << varName << " = (";
            for (size_t i = 0; i < buf.size(); ++i) {
                std::ostringstream ss;
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(buf[i]);
                out << ss.str();
                if (i + 1 < buf.size()) {
                    out << ",";
                }
                if (ctr++ == perLine) {
                    out << "\n";
                    ctr = 1;
                }
            }
            out << ");\n";
            break;
        }
        case 7: { // C#
            out << "byte[] " << varName
                << " = new byte[] {\n";
            for (size_t i = 0; i < buf.size(); ++i) {
                std::ostringstream ss;
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(buf[i]);
                out << ss.str();
                if (i + 1 < buf.size()) {
                    out << ",";
                    if (ctr++ == perLine) {
                        out << "\n";
                        ctr = 1;
                    }
                }
            }
            out << "};\n";
            break;
        }
        case 8: { // Plain hex string
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str();
            }
            out << "\n";
            break;
        }
        case 9: { // UUID list
            std::vector<unsigned char> tmp = buf;
            size_t pad = (16 - tmp.size() % 16) % 16;
            tmp.insert(tmp.end(), pad, 0);
            for (size_t off = 0; off < tmp.size(); off += 16) {
                std::ostringstream uuid;
                auto hex2 = [&](int i){
                    std::ostringstream s;
                    s << std::hex << std::setw(2) << std::setfill('0')
                      << int(tmp[off+i]);
                    return s.str();
                };
                auto hex4 = [&](int i){
                    std::ostringstream s;
                    int v = (tmp[off+i] << 8) | tmp[off+i+1];
                    s << std::hex << std::setw(4) << std::setfill('0') << v;
                    return s.str();
                };
                uuid << hex4(0)<<hex4(2) << "-" << hex2(4)<<hex2(5)
                     << "-" << hex2(6)<<hex2(7) << "-" << hex2(8)<<hex2(9)
                     << "-" << hex4(10)<<hex4(12);
                out << uuid.str() << "\n";
            }
            break;
        }
        case 10: { // Go
            out << "var " << varName << " = []byte{\n";
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str() << ",";
                if (ctr++ == perLine) {
                    out << "\n";
                    ctr = 1;
                }
            }
            out << "}\n";
            break;
        }
        case 11: { // Rust
            out << "let " << varName << ": &[u8] = &[\n";
            for (unsigned char b : buf) {
                std::ostringstream ss;
                ss << "0x" << std::hex << std::setw(2) << std::setfill('0')
                   << int(b);
                out << ss.str() << ",";
                if (ctr++ == perLine) {
                    out << "\n";
                    ctr = 1;
                }
            }
            out << "];\n";
            break;
        }
        default:
            throw std::runtime_error("Unknown format");
    }
    return out.str();
}

}
