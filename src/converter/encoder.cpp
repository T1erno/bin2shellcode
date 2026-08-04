#include "converter/encoder.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace b2s {

static const char *B64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/";

static std::string encodeBase64String(const std::vector<unsigned char> &in) {
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

EncodedResult
RawEncoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::Raw;
  res.bytes = bytes;
  for (unsigned char b : bytes) {
    std::ostringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << int(b);
    res.tokens.push_back(ss.str());
  }
  return res;
}

EncodedResult
Base64Encoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::Base64;
  res.bytes = bytes;
  res.rawString = encodeBase64String(bytes);
  return res;
}

EncodedResult
IPv4Encoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::IPv4;
  res.bytes = bytes;

  // IPv4 requires 4 bytes per IP address, padded with 0
  std::vector<unsigned char> tmp = bytes;
  size_t pad = (4 - tmp.size() % 4) % 4;
  tmp.insert(tmp.end(), pad, 0);

  for (size_t i = 0; i < tmp.size(); i += 4) {
    std::ostringstream ip;
    ip << int(tmp[i]) << "." << int(tmp[i + 1]) << "." << int(tmp[i + 2]) << "."
       << int(tmp[i + 3]);
    res.tokens.push_back(ip.str());
  }
  return res;
}

EncodedResult
MACEncoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::MAC;
  res.bytes = bytes;

  // MAC requires 6 bytes per address, padded with 0
  std::vector<unsigned char> tmp = bytes;
  size_t pad = (6 - tmp.size() % 6) % 6;
  tmp.insert(tmp.end(), pad, 0);

  for (size_t i = 0; i < tmp.size(); i += 6) {
    std::ostringstream mac;
    for (size_t j = 0; j < 6; ++j) {
      mac << std::hex << std::setw(2) << std::setfill('0') << int(tmp[i + j]);
      if (j < 5) {
        mac << "-";
      }
    }
    res.tokens.push_back(mac.str());
  }
  return res;
}

EncodedResult
UUIDEncoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::UUID;
  res.bytes = bytes;

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
         << hex2(7) << '-' << hex2(8) << hex2(9) << '-' << hex4(10) << hex4(12);
    res.tokens.push_back(uuid.str());
  }
  return res;
}

EncodedResult
HexEncoder::encode(const std::vector<unsigned char> &bytes) const {
  EncodedResult res;
  res.type = CipherType::Hex;
  res.bytes = bytes;

  std::ostringstream ss;
  ss << std::hex << std::setfill('0');
  for (unsigned char b : bytes) {
    ss << std::setw(2) << int(b);
  }
  res.rawString = ss.str();
  return res;
}

std::unique_ptr<Encoder> createEncoder(CipherType type) {
  switch (type) {
  case CipherType::Raw:
    return std::make_unique<RawEncoder>();
  case CipherType::Base64:
    return std::make_unique<Base64Encoder>();
  case CipherType::IPv4:
    return std::make_unique<IPv4Encoder>();
  case CipherType::MAC:
    return std::make_unique<MACEncoder>();
  case CipherType::UUID:
    return std::make_unique<UUIDEncoder>();
  case CipherType::Hex:
    return std::make_unique<HexEncoder>();
  }
  throw std::invalid_argument("Unknown cipher type");
}

} // namespace b2s
