// The textual form of a principal, without traps or exceptions.

#include "candid_principal_text.h"

#include "hash-library/crc32.h"

#include <algorithm>
#include <array>

static std::array<uint8_t, 4> crc32_be(const std::vector<uint8_t> &data) {
  CRC32 crc;
  crc.add(data.data(), data.size());
  std::array<uint8_t, 4> out{};
  crc.getHash(out.data());
  return out;
}

static const char *ALPHABET = "abcdefghijklmnopqrstuvwxyz234567";

std::string CandidPrincipalText::encode(const std::vector<uint8_t> &data) {
  std::array<uint8_t, 4> crc = crc32_be(data);
  std::vector<uint8_t> bytes(crc.begin(), crc.end());
  bytes.insert(bytes.end(), data.begin(), data.end());

  // Base32, lower case, no padding
  std::string b32;
  uint32_t buffer = 0;
  int bits = 0;
  for (uint8_t c : bytes) {
    buffer = (buffer << 8) | c;
    bits += 8;
    while (bits >= 5) {
      b32.push_back(ALPHABET[(buffer >> (bits - 5)) & 31]);
      bits -= 5;
    }
  }
  if (bits > 0) b32.push_back(ALPHABET[(buffer << (5 - bits)) & 31]);

  // Groups of 5, separated by '-'
  std::string out;
  for (size_t i = 0; i < b32.size(); ++i) {
    if (i > 0 && i % 5 == 0) out.push_back('-');
    out.push_back(b32[i]);
  }
  return out;
}

bool CandidPrincipalText::decode(const std::string &text,
                                 std::vector<uint8_t> &data, std::string &err) {
  data.clear();
  std::string b32;
  for (size_t i = 0; i < text.size(); ++i) {
    char c = text[i];
    if (i % 6 == 5) {
      if (c != '-') {
        err = "Principal: text should be separated by - every 5 characters";
        return false;
      }
      continue;
    }
    if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    b32.push_back(c);
  }

  std::vector<uint8_t> bytes;
  uint32_t buffer = 0;
  int bits = 0;
  for (char c : b32) {
    uint32_t v;
    if (c >= 'a' && c <= 'z') v = uint32_t(c - 'a');
    else if (c >= '2' && c <= '7') v = uint32_t(c - '2' + 26);
    else {
      err = "Principal: text must be valid base32";
      return false;
    }
    buffer = (buffer << 5) | v;
    bits += 5;
    if (bits >= 8) {
      bytes.push_back(uint8_t(buffer >> (bits - 8)));
      bits -= 8;
    }
    buffer &= (1u << bits) - 1u;
  }
  if (bytes.size() < 4) {
    err = "Principal: text is too short";
    return false;
  }
  data.assign(bytes.begin() + 4, bytes.end());
  if (data.size() > 29) {
    err = "Principal: more than 29 bytes";
    return false;
  }
  std::array<uint8_t, 4> crc = crc32_be(data);
  if (!std::equal(crc.begin(), crc.end(), bytes.begin())) {
    err = "Principal: the CRC32 checksum does not match";
    return false;
  }
  // The canonical text only: no padding bits set, no other grouping
  if (encode(data) != text) {
    err = "Principal: not the canonical text of its bytes";
    return false;
  }
  return true;
}
