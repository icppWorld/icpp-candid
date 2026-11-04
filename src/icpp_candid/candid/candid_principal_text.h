// The textual form of a principal, without traps or exceptions.
//
//     https://internetcomputer.org/docs/references/id-encoding-spec
//
// Encode(data) := Group(LowerCase(Base32(CRC32(data) || data)))

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class CandidPrincipalText {
public:
  // The principal's text, e.g. "aaaaa-aa" for no bytes
  static std::string encode(const std::vector<uint8_t> &data);
  // False with an error for a text that is not a valid principal: bad
  // grouping, bad base32, a wrong checksum, or more than 29 bytes
  static bool decode(const std::string &text, std::vector<uint8_t> &data,
                     std::string &err);
};
