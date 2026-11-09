// An arbitrary-precision integer for Candid's nat and int.
//
// Candid's nat and int are unbounded: a value on the wire is a LEB128 of any
// length. CandidBigInt holds such a value exactly, so the decoder never has to
// truncate or reject a number because it is large. A typed binding converts it
// to a C++ integer, and traps cleanly when it does not fit.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class CandidBigInt {
public:
  CandidBigInt() = default;
  static CandidBigInt from_u128(__uint128_t v);
  static CandidBigInt from_i128(__int128_t v);

  // LEB128 (nat) and SLEB128 (int) groups: the 7-bit payloads, least
  // significant first, without the continuation bits. Linear in the length.
  static CandidBigInt from_leb_groups(const std::vector<uint8_t> &groups);
  static CandidBigInt from_sleb_groups(const std::vector<uint8_t> &groups);

  // Decimal digits, or hex digits after "0x", with optional '_' separators
  // between digits and an optional leading '+' or '-'. False on a bad form.
  static bool parse(const std::string &s, CandidBigInt &out);

  bool is_negative() const { return m_neg; }
  bool is_zero() const { return m_limbs.empty(); }

  // Conversions that fail when the value does not fit
  bool to_u128(__uint128_t &v) const;
  bool to_i128(__int128_t &v) const;
  bool to_u64(uint64_t &v) const;
  bool to_i64(int64_t &v) const;

  std::string to_string() const;

  // LEB128 (nat) and SLEB128 (int), appended to out: the shortest form, as
  // Candid writes them. to_leb128 is false for a negative value.
  bool to_leb128(std::vector<uint8_t> &out) const;
  void to_sleb128(std::vector<uint8_t> &out) const;

  bool operator==(const CandidBigInt &o) const {
    return m_neg == o.m_neg && m_limbs == o.m_limbs;
  }
  bool operator!=(const CandidBigInt &o) const { return !(*this == o); }

private:
  bool m_neg{false};
  std::vector<uint32_t> m_limbs; // magnitude, least significant limb first

  void normalize();
  void mul_small(uint32_t m);
  void add_small(uint32_t a);
  uint32_t divmod_small(uint32_t d);
  static CandidBigInt from_groups(const std::vector<uint8_t> &groups);
  bool bit(size_t k) const;
  size_t bit_length() const;
  void write_groups(std::vector<uint8_t> &out, bool sleb, bool invert) const;
};
