// An arbitrary-precision integer for Candid's nat and int.

#include "candid_bigint.h"

#include <algorithm>

void CandidBigInt::normalize() {
  while (!m_limbs.empty() && m_limbs.back() == 0)
    m_limbs.pop_back();
  if (m_limbs.empty()) m_neg = false;
}

CandidBigInt CandidBigInt::from_u128(__uint128_t v) {
  CandidBigInt r;
  while (v != 0) {
    r.m_limbs.push_back(static_cast<uint32_t>(v));
    v >>= 32;
  }
  return r;
}

CandidBigInt CandidBigInt::from_i128(__int128_t v) {
  bool neg = v < 0;
  // Negate in the unsigned domain, so INT128_MIN does not overflow
  __uint128_t mag =
      neg ? __uint128_t(0) - static_cast<__uint128_t>(v) : __uint128_t(v);
  CandidBigInt r = from_u128(mag);
  r.m_neg = neg && !r.m_limbs.empty();
  return r;
}

// Place 7-bit groups at bit offsets 0, 7, 14, ...: linear in the length.
CandidBigInt CandidBigInt::from_groups(const std::vector<uint8_t> &groups) {
  CandidBigInt r;
  size_t nbits = groups.size() * 7;
  r.m_limbs.assign(nbits / 32 + 1, 0);
  for (size_t i = 0; i < groups.size(); ++i) {
    uint64_t g = groups[i] & 0x7f;
    size_t bit = i * 7;
    size_t limb = bit / 32;
    size_t shift = bit % 32;
    uint64_t v = g << shift;
    r.m_limbs[limb] |= static_cast<uint32_t>(v);
    if ((v >> 32) != 0) r.m_limbs[limb + 1] |= static_cast<uint32_t>(v >> 32);
  }
  r.normalize();
  return r;
}

CandidBigInt CandidBigInt::from_leb_groups(const std::vector<uint8_t> &groups) {
  return from_groups(groups);
}

CandidBigInt
CandidBigInt::from_sleb_groups(const std::vector<uint8_t> &groups) {
  CandidBigInt r = from_groups(groups);
  if (groups.empty() || (groups.back() & 0x40) == 0) return r;

  // Negative: the value is v - 2^(7n), so its magnitude is 2^(7n) - v.
  // Compute it as the two's complement of v over 7n bits.
  size_t nbits = groups.size() * 7;
  size_t nlimbs = nbits / 32 + 1;
  r.m_limbs.resize(nlimbs, 0);
  for (auto &l : r.m_limbs)
    l = ~l;
  // Clear the bits above 7n
  size_t top_bits = nbits % 32;
  r.m_limbs[nlimbs - 1] &= (top_bits == 0) ? 0u : ((1u << top_bits) - 1u);
  if (top_bits == 0) r.m_limbs[nlimbs - 1] = 0;
  r.m_neg = false;
  r.add_small(1);
  r.m_neg = true;
  r.normalize();
  return r;
}

void CandidBigInt::mul_small(uint32_t m) {
  uint64_t carry = 0;
  for (auto &l : m_limbs) {
    uint64_t v = uint64_t(l) * m + carry;
    l = static_cast<uint32_t>(v);
    carry = v >> 32;
  }
  if (carry != 0) m_limbs.push_back(static_cast<uint32_t>(carry));
}

void CandidBigInt::add_small(uint32_t a) {
  uint64_t carry = a;
  for (size_t i = 0; i < m_limbs.size() && carry != 0; ++i) {
    uint64_t v = uint64_t(m_limbs[i]) + carry;
    m_limbs[i] = static_cast<uint32_t>(v);
    carry = v >> 32;
  }
  if (carry != 0) m_limbs.push_back(static_cast<uint32_t>(carry));
}

uint32_t CandidBigInt::divmod_small(uint32_t d) {
  uint64_t rem = 0;
  for (size_t i = m_limbs.size(); i-- > 0;) {
    uint64_t cur = (rem << 32) | m_limbs[i];
    m_limbs[i] = static_cast<uint32_t>(cur / d);
    rem = cur % d;
  }
  normalize();
  return static_cast<uint32_t>(rem);
}

bool CandidBigInt::parse(const std::string &s, CandidBigInt &out) {
  out = CandidBigInt();
  size_t i = 0;
  bool neg = false;
  if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
    neg = s[i] == '-';
    ++i;
  }
  uint32_t base = 10;
  if (i + 1 < s.size() && s[i] == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X')) {
    base = 16;
    i += 2;
  }
  bool any_digit = false;
  bool prev_underscore = false;
  for (; i < s.size(); ++i) {
    char c = s[i];
    if (c == '_') {
      // '_' only between two digits
      if (!any_digit || prev_underscore) return false;
      prev_underscore = true;
      continue;
    }
    uint32_t d;
    if (c >= '0' && c <= '9') d = uint32_t(c - '0');
    else if (base == 16 && c >= 'a' && c <= 'f') d = uint32_t(c - 'a' + 10);
    else if (base == 16 && c >= 'A' && c <= 'F') d = uint32_t(c - 'A' + 10);
    else return false;
    out.mul_small(base);
    out.add_small(d);
    any_digit = true;
    prev_underscore = false;
  }
  if (!any_digit || prev_underscore) return false;
  out.normalize();
  out.m_neg = neg && !out.m_limbs.empty();
  return true;
}

bool CandidBigInt::to_u128(__uint128_t &v) const {
  if (m_neg || m_limbs.size() > 4) return false;
  v = 0;
  for (size_t i = m_limbs.size(); i-- > 0;)
    v = (v << 32) | m_limbs[i];
  return true;
}

bool CandidBigInt::to_i128(__int128_t &v) const {
  if (m_limbs.size() > 4) return false;
  __uint128_t mag = 0;
  for (size_t i = m_limbs.size(); i-- > 0;)
    mag = (mag << 32) | m_limbs[i];
  const __uint128_t limit = __uint128_t(1) << 127;
  if (m_neg) {
    if (mag > limit) return false;
    v = static_cast<__int128_t>(__uint128_t(0) - mag);
  } else {
    if (mag >= limit) return false;
    v = static_cast<__int128_t>(mag);
  }
  return true;
}

bool CandidBigInt::to_u64(uint64_t &v) const {
  __uint128_t w;
  if (!to_u128(w) || w > UINT64_MAX) return false;
  v = static_cast<uint64_t>(w);
  return true;
}

bool CandidBigInt::to_i64(int64_t &v) const {
  __int128_t w;
  if (!to_i128(w) || w > INT64_MAX || w < INT64_MIN) return false;
  v = static_cast<int64_t>(w);
  return true;
}

bool CandidBigInt::bit(size_t k) const {
  size_t limb = k / 32;
  return limb < m_limbs.size() && ((m_limbs[limb] >> (k % 32)) & 1u) != 0;
}

size_t CandidBigInt::bit_length() const {
  if (m_limbs.empty()) return 0;
  size_t n = (m_limbs.size() - 1) * 32;
  for (uint32_t top = m_limbs.back(); top != 0; top >>= 1)
    ++n;
  return n;
}

// The 7-bit groups of the magnitude's bits, least significant first. With
// invert, the bits are inverted: the two's complement of a negative number is
// the inverted bits of |x| - 1. Unsigned (LEB128), the last group is the one
// with no bits above it; signed (SLEB128), it must also carry the sign in its
// bit 6, so the bits above it all repeat that sign.
void CandidBigInt::write_groups(std::vector<uint8_t> &out, bool sleb,
                                bool invert) const {
  size_t len = bit_length();
  for (size_t i = 0;; ++i) {
    uint8_t group = 0;
    for (size_t j = 0; j < 7; ++j)
      if (bit(7 * i + j) != invert) group |= uint8_t(1u << j);
    bool sign_ok = !sleb || (((group & 0x40) != 0) == invert);
    bool done = 7 * (i + 1) >= len && sign_ok;
    out.push_back(done ? group : uint8_t(group | 0x80));
    if (done) return;
  }
}

bool CandidBigInt::to_leb128(std::vector<uint8_t> &out) const {
  if (m_neg) return false;
  write_groups(out, false, false);
  return true;
}

void CandidBigInt::to_sleb128(std::vector<uint8_t> &out) const {
  if (!m_neg) {
    write_groups(out, true, false);
    return;
  }
  // |x| - 1, a magnitude of at least 0
  CandidBigInt m = *this;
  m.m_neg = false;
  for (auto &l : m.m_limbs) {
    if (l-- != 0) break;
  }
  m.normalize();
  m.write_groups(out, true, true);
}

std::string CandidBigInt::to_string() const {
  if (m_limbs.empty()) return "0";
  CandidBigInt t = *this;
  t.m_neg = false;
  std::string digits;
  while (!t.m_limbs.empty()) {
    uint32_t chunk = t.divmod_small(1000000000u);
    for (int k = 0; k < 9; ++k) {
      digits.push_back(char('0' + chunk % 10));
      chunk /= 10;
      if (t.m_limbs.empty() && chunk == 0) break;
    }
  }
  while (digits.size() > 1 && digits.back() == '0')
    digits.pop_back();
  if (m_neg) digits.push_back('-');
  std::reverse(digits.begin(), digits.end());
  return digits;
}
