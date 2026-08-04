#ifndef ENCODER_H
#define ENCODER_H

#include <memory>
#include <string>
#include <vector>

namespace b2s {

enum class CipherType {
  Raw,
  Base64,
  IPv4,
  MAC,
  UUID,
  Hex
};

/**
 * Result of an encoding / cipher operation.
 */
struct EncodedResult {
  CipherType type{CipherType::Raw};
  std::string rawString;            // For Base64 or continuous hex string
  std::vector<std::string> tokens;  // For IPv4, MAC, UUID, or byte hexes
  std::vector<unsigned char> bytes; // Raw input bytes
};

/**
 * Base Class for Encoders / Ciphers.
 */
class Encoder {
public:
  virtual ~Encoder() = default;
  virtual EncodedResult encode(const std::vector<unsigned char> &bytes) const = 0;
  virtual CipherType getType() const = 0;
};

class RawEncoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::Raw; }
};

class Base64Encoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::Base64; }
};

class IPv4Encoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::IPv4; }
};

class MACEncoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::MAC; }
};

class UUIDEncoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::UUID; }
};

class HexEncoder : public Encoder {
public:
  EncodedResult encode(const std::vector<unsigned char> &bytes) const override;
  CipherType getType() const override { return CipherType::Hex; }
};

std::unique_ptr<Encoder> createEncoder(CipherType type);

} // namespace b2s

#endif // ENCODER_H
