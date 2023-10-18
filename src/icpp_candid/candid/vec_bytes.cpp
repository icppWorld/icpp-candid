//  Wrapper around a std::vector<std::byte>

#include "vec_bytes.h"

#include <cassert>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <bit>
#include <string>

#include <limits.h>
#include <limits>
#include <sstream>

#include "icpp_hooks.h"

#define INT_STR_SIZE (sizeof(int) * CHAR_BIT / 3 + 3)

VecBytes::VecBytes() { check_endian(); }

VecBytes::~VecBytes() {}

// TODO: use __int128_t
std::string VecBytes::int_to_hex(const int i, const std::string prefix) {
  const int LEN_STR = 80;
  char str[LEN_STR] = {'\0'};
  c_int_to_hex(str, (unsigned int)i);
  std::string s{prefix};
  return s.append(str);

  // TODO: Using stringstream causes too many globals
  //       600+, only 300 allowed   (Dec 15, 2022)
  // From: https://stackoverflow.com/a/5100745/5480536
  // std::stringstream stream;
  // stream << prefix
  //        << std::setfill('0') << std::setw(2)
  //        << std::hex << i;
  // return stream.str();
}

std::string VecBytes::byte_to_hex(const std::byte b, const std::string prefix) {
  return int_to_hex(int(b), prefix);
}

// TODO: just use char ?
std::string VecBytes::int_to_dec(const int i, const int width) {
  char str[INT_STR_SIZE] = {'\0'};
  c_int_to_dec(str, INT_STR_SIZE, i);
  std::string s;
  for (size_t i = 0; i < (width - strlen(str)); ++i) {
    s.append(" ");
  }
  s.append(str);
  return std::string{s};

  // TODO: Using stringstream causes too many globals
  //       600+, only 300 allowed   (Dec 15, 2022)
  // stream << std::setfill(' ') << std::setw(width)
  //        << std::dec << i;
  // return stream.str();
}

std::string VecBytes::byte_to_char(const std::byte b) {
  return std::string{(const char)(int)b};
}

// TODO: use __int128_t
std::string VecBytes::byte_to_dec(const std::byte b, const int width) {
  return int_to_dec(int(b), width);
}

void VecBytes::c_int_to_hex(char *str, unsigned int x) {
  char arr[17] = "0123456789abcdef";
  char *p = (char *)&x;
  int j = 0;
  bool found_non_zero = false;
  for (size_t i = 0; i < 8; ++i) {
    unsigned char ch = (x >> ((32 - 4) - i * 4)) & 0xF;
    if (found_non_zero || i > 5) {

      str[j] = arr[ch];

      if (str[j] != '0') {
        found_non_zero = true;
        if (i < 6) {
          ICPP_HOOKS::trap(
              "PROGRAM ERROR in int_to_hex. Hex string has more than 2 "
              "characters?");
        }
      }
      ++j;
    }
  }
  str[j] = '\0';
}

// https://stackoverflow.com/a/36392305/5480536
// Convert unsigned int to dec string, without using sprintf
void VecBytes::c_int_to_dec(char *str, size_t size, int x) {
  char buf[INT_STR_SIZE];
  char *p = &buf[INT_STR_SIZE - 1];
  *p = '\0';
  int i = x;

  do {
    *(--p) = abs(i % 10) + '0';
    i /= 10;
  } while (i);

  if (x < 0) {
    *(--p) = '-';
  }
  size_t len = (size_t)(&buf[INT_STR_SIZE] - p);
  if (len > size) {
    ICPP_HOOKS::trap(
        "PROGRAM ERROR in c_int_to_dec - Buffer to small to store decimal "
        "representation of int");
  }
  memcpy(str, p, len);
}

void VecBytes::append_didl() {
  append_byte((std::byte)'D');
  append_byte((std::byte)'I');
  append_byte((std::byte)'D');
  append_byte((std::byte)'L');
}

// ------------------------------------------------------------------------------
// append_uleb128

// append bool as uleb128
void VecBytes::append_uleb128(const bool &v) {
  if (v) append_uleb128(__uint128_t(1));
  else append_uleb128(__uint128_t(0));
}

// append __uint128_t as uleb128
void VecBytes::append_uleb128(const __uint128_t &v) {
  uint8_t Bytes[1024];
  auto numBytes = encode_uleb128(v, Bytes);
  assert(numBytes < sizeof(Bytes));
  for (size_t i = 0; i < numBytes; ++i) {
    append_byte((std::byte)Bytes[i]);
  }
}

// ------------------------------------------------------------------------------
// append_sleb128

// append __int128_t as sleb128
void VecBytes::append_sleb128(const __int128_t &v) {
  uint8_t Bytes[1024];
  auto numBytes = encode_sleb128(v, Bytes);
  assert(numBytes < sizeof(Bytes));
  for (size_t i = 0; i < numBytes; ++i) {
    append_byte((std::byte)Bytes[i]);
  }
}

// Decode with uleb128, starting at & updating offset
bool VecBytes::parse_uleb128(__uint128_t &offset, __uint128_t &v,
                             __uint128_t &numbytes, std::string &parse_error) {
  __uint128_t len = m_vec.size() - offset;

  uint8_t *buf = &m_vec_uint8_t[offset];
  uint8_t *buf_end = &m_vec_uint8_t[offset + len];

  parse_error = "";
  v = decode_uleb128(buf, &numbytes, buf_end, parse_error);
  offset += numbytes;

  if (parse_error.size() > 0) {
    return true;
  }

  return false;
}

// Decode bytes with sleb128, starting at & updating offset
bool VecBytes::parse_sleb128(__uint128_t &offset, __int128_t &v,
                             __uint128_t &numbytes, std::string &parse_error) {
  __uint128_t len = m_vec.size() - offset;

  uint8_t *buf = &m_vec_uint8_t[offset];
  uint8_t *buf_end = &m_vec_uint8_t[offset + len];

  parse_error = "";
  v = decode_sleb128(buf, &numbytes, buf_end, parse_error);
  offset += numbytes;

  if (parse_error.size() > 0) {
    return true;
  }

  return false;
}

// Parse n bytes, starting at & updating offset
bool VecBytes::parse_bytes(__uint128_t &offset, std::vector<std::byte> &v,
                           __uint128_t &n, __uint128_t &numbytes,
                           std::string &parse_error) {
  parse_error = "";
  numbytes = 0;

  __uint128_t len = m_vec.size() - offset;

  if (n > len) {
    parse_error =
        "Not enough bytes left. The remaining bytes in the byte stream on wire is ";
    parse_error.append(ICPP_HOOKS::to_string_128(len));
    parse_error.append(", but specified number to parse is ");
    parse_error.append(ICPP_HOOKS::to_string_128(n));
    return true;
  }

  std::copy(m_vec.begin() + offset, m_vec.begin() + offset + n,
            std::back_inserter(v));

  numbytes = n;
  offset += numbytes;

  return false;
}

// Parse n bytes, starting at & updating offset
bool VecBytes::parse_bytes(__uint128_t &offset, std::vector<uint8_t> &v,
                           __uint128_t &n, __uint128_t &numbytes,
                           std::string &parse_error) {
  parse_error = "";
  numbytes = 0;

  __uint128_t len = m_vec.size() - offset;

  if (n > len) {
    parse_error =
        "Not enough bytes left. The remaining bytes in the byte stream on wire is ";
    parse_error.append(ICPP_HOOKS::to_string_128(len));
    parse_error.append(", but specified number to parse is ");
    parse_error.append(ICPP_HOOKS::to_string_128(n));
    return true;
  }

  std::copy(m_vec_uint8_t.begin() + offset, m_vec_uint8_t.begin() + offset + n,
            std::back_inserter(v));

  numbytes = n;
  offset += numbytes;

  return false;
}

void VecBytes::store(const uint8_t *bytes, const uint32_t num_bytes) {
  clear();
  append_bytes(bytes, num_bytes);
}

// Initializes m_vec from a bytes array string in hex format.
// candid_in example: "4449444c0001710d432b2b20446576656c6f706572"
// generated with   : $ didc encode '("C++ Developer")' -t'(text)'
void VecBytes::store_hex_string(const std::string hex_string) {
  // Get a vector of strings with the hex tokens, by splitting out every 2 chars
  std::vector<std::string> hex_tokens;
  for (size_t i = 0; i < hex_string.size(); i += 2)
    hex_tokens.push_back(hex_string.substr(i, 2));

  store_hex_tokens(hex_tokens);
}

// convert a vector of tokens into bytes and initialize m_vec with them
void VecBytes::store_hex_tokens(std::vector<std::string> hex_tokens) {
  clear();
  for (std::string s : hex_tokens) {
    int n;

    // TODO: Using istringstream causes too many globals
    //       600+, only 300 allowed   (Jan 15, 2023)
    // std::istringstream(s) >> std::hex >> n;

    // https://stackoverflow.com/a/45353247/5480536
    n = std::stoi(s, nullptr, 16);

    append_byte((const std::byte)n);
  }
}

void VecBytes::append_byte(std::byte b) {
  m_vec.push_back(b);

  // Maintain the same vector as a uint8_t type
  int n = (int)b;
  m_vec_uint8_t.push_back((uint8_t)n);
}

void VecBytes::append_bytes(const uint8_t *bytes, const uint32_t num_bytes) {
  for (size_t i = 0; i < num_bytes; ++i) {
    append_byte((const std::byte)bytes[i]);
  }
}

void VecBytes::trap_if_vec_does_not_start_with_DIDL() {
  std::string s;
  s.append(byte_to_char(m_vec[0]));
  s.append(byte_to_char(m_vec[1]));
  s.append(byte_to_char(m_vec[2]));
  s.append(byte_to_char(m_vec[3]));
  if (s != "DIDL") {
    ICPP_HOOKS::trap("ERROR: Message does not start with DIDL");
  }
}

// Returns the bytes of the vec as a hex string
std::string VecBytes::as_hex_string() {
  std::string s_hex;
  for (std::byte b : m_vec) {
    s_hex.append(byte_to_hex(b, ""));
  }
  return s_hex;
}

// Prints the bytes of the vec in hex, dec & bits
void VecBytes::debug_print() {
  ICPP_HOOKS::debug_print("  hex, decimal, char");

  std::string s_hex;
  for (std::byte b : m_vec) {
    std::string s;
    s.append("   ");
    s.append(byte_to_hex(b, ""));
    s.append(", ");
    s.append(byte_to_dec(b, 8));
    s.append(", ");
    s.append(byte_to_char(b));
    s_hex.append(byte_to_hex(b, ""));

    ICPP_HOOKS::debug_print(s); // Print a nice table: hex, decimal, char
  }
  ICPP_HOOKS::debug_print(
      s_hex); // Print the whole thing in hex on one line, for didc decode !
}

void VecBytes::clear() {
  m_vec.clear();
  m_vec_uint8_t.clear();
}

// Helper function to assign values to __int128_t from strings
// https://stackoverflow.com/a/45608851/5480536
__int128_t VecBytes::atoint128_t(const std::string &in) {
  __int128_t res = 0;
  size_t i = 0;
  bool sign = false;

  if (in[i] == '-') {
    ++i;
    sign = true;
  }

  if (in[i] == '+') {
    ++i;
  }

  for (; i < in.size(); ++i) {
    const char c = in[i];
    if (not std::isdigit(c))
      ICPP_HOOKS::trap(std::string("Non-numeric character: ") + c);

    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= 10;

    res += c - '0';
    // https://stackoverflow.com/a/6472982/5480536
    if (res < c - '0') {
      ICPP_HOOKS::trap(
          std::string(
              "ERROR: Overflow - Cannot convert string to __uint128t.\n       Number is too big: ") +
          in);
    }
  }

  if (sign) {
    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= -1;
  }

  return res;
}

__uint128_t VecBytes::atouint128_t(const std::string &in) {
  __uint128_t res = 0;
  size_t i = 0;

  if (in[i] == '+') {
    ++i;
  }

  for (; i < in.size(); ++i) {
    const char c = in[i];
    if (not std::isdigit(c))
      ICPP_HOOKS::trap(std::string("Non-numeric character: ") + c);

    // TODO: implement multiplication overflow protection: https://stackoverflow.com/a/1815371/5480536
    res *= 10;

    res += c - '0';
    // https://stackoverflow.com/a/6472982/5480536
    if (res < c - '0') {
      ICPP_HOOKS::trap(
          std::string(
              "ERROR: Overflow - Cannot convert string to __uint128t.\n       Number is too big: ") +
          in);
    }
  }

  return res;
}

// Utility function to encode a SLEB128 value to a buffer.
// Returns the length in bytes of the encoded value.
unsigned VecBytes::encode_sleb128(__int128_t Value, uint8_t *p) {
  uint8_t *orig_p = p;
  unsigned Count = 0;
  bool More;
  do {
    uint8_t Byte = Value & 0x7f;
    // NOTE: this assumes that this signed shift is an arithmetic right shift.
    Value >>= 7;
    More = !((((Value == 0) && ((Byte & 0x40) == 0)) ||
              ((Value == -1) && ((Byte & 0x40) != 0))));
    Count++;
    if (More)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    *p++ = Byte;
  } while (More);
  return (unsigned)(p - orig_p);
}

// Utility function to encode a ULEB128 value to a buffer.
// Returns the length in bytes of the encoded value.
unsigned VecBytes::encode_uleb128(__uint128_t Value, uint8_t *p) {
  uint8_t *orig_p = p;
  unsigned Count = 0;
  do {
    uint8_t Byte = Value & 0x7f;
    Value >>= 7;
    Count++;
    if (Value != 0)
      Byte |= 0x80; // Mark this byte to show that more bytes will follow.
    *p++ = Byte;
  } while (Value != 0);
  return (unsigned)(p - orig_p);
}

// Utility function to decode a ULEB128 value.
__uint128_t VecBytes::decode_uleb128(const uint8_t *p, __uint128_t *n,
                                     const uint8_t *end, std::string &error) {
  const uint8_t *orig_p = p;
  __uint128_t Value = 0;
  unsigned Shift = 0;
  do {
    if (p == end) {
      error.append("ERROR: malformed uleb128, extends past end");
      *n = (unsigned)(p - orig_p);
      return 0;
    }
    __uint128_t Slice = *p & 0x7f;
    if ((Shift >= 128 && Slice != 0) || Slice << Shift >> Shift != Slice) {
      error.append("uleb128 too big for __uint128_t");
      *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value += Slice << Shift;
    Shift += 7;
  } while (*p++ >= 128);
  *n = (unsigned)(p - orig_p);
  return Value;
}

// Utility function to decode a SLEB128 value.
__int128_t VecBytes::decode_sleb128(const uint8_t *p, __uint128_t *n,
                                    const uint8_t *end, std::string &error) {
  const uint8_t *orig_p = p;
  // TODO: update this function for an __int128_t
  int64_t Value = 0;
  unsigned Shift = 0;
  uint8_t Byte;
  do {
    if (p == end) {
      error.append("malformed sleb128, extends past end");
      if (n) *n = (unsigned)(p - orig_p);
      return 0;
    }
    Byte = *p;
    uint64_t Slice = Byte & 0x7f;
    if ((Shift >= 64 && Slice != (Value < 0 ? 0x7f : 0x00)) ||
        (Shift == 63 && Slice != 0 && Slice != 0x7f)) {
      error.append("sleb128 too big for int64");
      if (n) *n = (unsigned)(p - orig_p);
      return 0;
    }
    Value |= Slice << Shift;
    Shift += 7;
    ++p;
  } while (Byte >= 128);
  // Sign extend negative numbers if needed.
  if (Shift < 64 && (Byte & 0x40)) Value |= (-1ULL) << Shift;
  if (n) *n = (unsigned)(p - orig_p);
  return __int128_t(Value);
}

// Utility function to get the size of the ULEB128-encoded value.
unsigned VecBytes::getULEB128Size(uint64_t Value) {
  unsigned Size = 0;
  do {
    Value >>= 7;
    Size += sizeof(int8_t);
  } while (Value);
  return Size;
}

// Utility function to get the size of the SLEB128-encoded value.
unsigned VecBytes::getSLEB128Size(int64_t Value) {
  unsigned Size = 0;
  int Sign = Value >> (8 * sizeof(Value) - 1);
  bool IsMore;

  do {
    unsigned Byte = Value & 0x7f;
    Value >>= 7;
    IsMore = Value != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += sizeof(int8_t);
  } while (IsMore);
  return Size;
}

// Utility function to check if we are running on little-endian
bool VecBytes::is_little_endian() {
  if constexpr (std::endian::native == std::endian::little) return true;
  else return false;
}

// Utility function to check if we are running on big-endian
bool VecBytes::is_big_endian() {
  if constexpr (std::endian::native == std::endian::big) return true;
  else return false;
}

// Utility function to check if we are running on mixed-endian
bool VecBytes::is_mixed_endian() {
  if (is_little_endian() || is_big_endian()) return false;
  else return true;
}

// Utility function to check if we are running on a system with IEEE 754 floats
bool VecBytes::is_float_ieee754() {
  if constexpr (std::numeric_limits<double>::is_iec559 == true) return true;
  else return false;
}

void VecBytes::check_endian() {
  if (is_little_endian()) m_endian_type = "little";
  else if (is_big_endian()) {
    m_endian_type = "big";
  } else if (is_mixed_endian()) {
    m_endian_type = "mixed";
  } else {
    ICPP_HOOKS::trap("Could not determine endianness of architecture. ");
  }
}

void VecBytes::trap(const std::string &msg) { ICPP_HOOKS::trap(msg); }

// -----------------------------------------------------------------------------
// template definitions with Explicit Template Instantiations
// Doing it this way avoids the COMDAT warnings & multiple definitions errors

template <typename T>
  requires MyFixedWidthInts<T>
void VecBytes::append_int_fixed_width(const T &v) {
  uint8_t *bytes{nullptr};
  if (is_little_endian()) bytes = (uint8_t *)&v;
  else
    // Probably best to do a memcpy and then a byteswap
    trap(
        "ERROR: append_int_fixed_width not yet implemented on big endian architecture");

  append_bytes(bytes, sizeof(v));
}
// clang-format off
template void VecBytes::append_int_fixed_width<int8_t>  (const int8_t   &v);
template void VecBytes::append_int_fixed_width<int16_t> (const int16_t  &v);
template void VecBytes::append_int_fixed_width<int32_t> (const int32_t  &v);
template void VecBytes::append_int_fixed_width<int64_t> (const int64_t  &v);
template void VecBytes::append_int_fixed_width<uint8_t> (const uint8_t  &v);
template void VecBytes::append_int_fixed_width<uint16_t>(const uint16_t &v);
template void VecBytes::append_int_fixed_width<uint32_t>(const uint32_t &v);
template void VecBytes::append_int_fixed_width<uint64_t>(const uint64_t &v);
// clang-format on

// --
template <typename T>
  requires MyFloats<T>
void VecBytes::append_float_ieee754(const T &v) {
  if (is_float_ieee754()) {
    // https://github.com/dfinity/candid/blob/master/spec/Candid.md#floating-point-numbers
    // Floating-point values are represented in IEEE 754 binary format and are
    // supported in single precision (32 bit) and double precision (64 bit).
    // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
    // M(z : float<N>) = f<N>(z)
    uint32_t off_now = 0;
    uint8_t *bytes = (uint8_t *)&v;
    append_bytes(bytes, sizeof(v));
  } else
    trap("ERROR in " + std::string(__func__) +
         ". The encoding method requires that your system is using IEEE "
         "754 floating point which it does not. "
         "(std::numeric_limits<double>::is_iec559 == false)");
}
// clang-format off
template void VecBytes::append_float_ieee754<float>  (const float &v);
template void VecBytes::append_float_ieee754<double> (const double &v);
// clang-format on

// --
template <typename T>
  requires MyFixedWidthInts<T>
bool VecBytes::parse_int_fixed_width(__uint128_t &offset, T &v,
                                     std::string &parse_error) {
  __uint128_t len = m_vec.size() - offset;

  uint8_t *buf = &m_vec_uint8_t[offset];
  uint8_t *buf_end = &m_vec_uint8_t[offset + len];

  parse_error = "";
  std::memcpy(&v, buf, sizeof(v));

  offset += sizeof(v);

  return false;
}

// clang-format off
template bool VecBytes::parse_int_fixed_width<int8_t>  (__uint128_t &offset, int8_t   &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<int16_t> (__uint128_t &offset, int16_t  &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<int32_t> (__uint128_t &offset, int32_t  &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<int64_t> (__uint128_t &offset, int64_t  &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<uint8_t> (__uint128_t &offset, uint8_t  &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<uint16_t>(__uint128_t &offset, uint16_t &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<uint32_t>(__uint128_t &offset, uint32_t &v, std::string &parse_error);
template bool VecBytes::parse_int_fixed_width<uint64_t>(__uint128_t &offset, uint64_t &v, std::string &parse_error);
// clang-format on

// --
template <typename T>
  requires MyFloats<T>
bool VecBytes::parse_float_ieee754(__uint128_t &offset, T &v,
                                   std::string &parse_error) {
  if (is_float_ieee754()) {
    // https://github.com/dfinity/candid/blob/master/spec/Candid.md#floating-point-numbers
    // Floating-point values are represented in IEEE 754 binary format and are
    // supported in single precision (32 bit) and double precision (64 bit).
    // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
    // M(z : float<N>) = f<N>(z)
    __uint128_t len = m_vec.size() - offset;

    uint8_t *buf = &m_vec_uint8_t[offset];
    uint8_t *buf_end = &m_vec_uint8_t[offset + len];

    parse_error = "";
    std::memcpy(&v, buf, sizeof(v));

    offset += sizeof(v);

  } else
    trap("ERROR in " + std::string(__func__) +
         ". The encoding method requires that your system is using IEEE "
         "754 floating point which it does not. "
         "(std::numeric_limits<double>::is_iec559 == false)");

  return false;
}
// clang-format off
template bool VecBytes::parse_float_ieee754<float>  (__uint128_t &offset, float  &v, std::string &parse_error);
template bool VecBytes::parse_float_ieee754<double> (__uint128_t &offset, double &v, std::string &parse_error);
// clang-format on
