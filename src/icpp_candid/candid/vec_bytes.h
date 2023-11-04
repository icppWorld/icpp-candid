//  Wrapper around a std::vector<std::byte>

#pragma once

#include <cstdint>
#include <cstring>

#include <string>
#include <vector>

template <typename T>
concept MyFixedWidthInts =
    std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
    std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> ||
    std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> ||
    std::is_same_v<T, int32_t> || std::is_same_v<T, int64_t>;
;

template <typename T>
concept MyFloats = std::is_same_v<T, float> || std::is_same_v<T, double>;
;

class VecBytes {
public:
  // Constructors
  VecBytes();

  // Destructor
  ~VecBytes();

  // Static utils
  static __int128_t atoint128_t(const std::string &in);
  static __uint128_t atouint128_t(const std::string &in);
  static std::string byte_to_hex(const std::byte b,
                                 const std::string prefix = "0x");
  static std::string byte_to_dec(const std::byte b, const int width = 8);
  static std::string byte_to_char(const std::byte b);
  static std::string int_to_hex(const int i, const std::string prefix = "0x");
  static std::string int_to_dec(const int i, const int width = 8);

  // C style utils
  static void c_int_to_hex(char *str, unsigned int x);
  static void c_int_to_dec(char *str, size_t size, int x);

  // leb128 methods
  unsigned encode_sleb128(__int128_t Value, uint8_t *p);
  template <class T> unsigned encode_sleb128(T, uint8_t *p) = delete;

  unsigned encode_uleb128(__uint128_t Value, uint8_t *p);
  template <class T> unsigned encode_uleb128(T, uint8_t *p) = delete;

  __uint128_t decode_uleb128(const uint8_t *p, __uint128_t *n,
                             const uint8_t *end, std::string &error);
  template <class T>
  T decode_uleb128(const uint8_t *p, __uint128_t *n, const uint8_t *end,
                   std::string &error) = delete;

  __int128_t decode_sleb128(const uint8_t *p, __uint128_t *n,
                            const uint8_t *end, std::string &error);
  template <class T>
  T decode_sleb128(const uint8_t *p, __uint128_t *n, const uint8_t *end,
                   std::string &error) = delete;
  /// Utility function to get the size of the ULEB128-encoded value.
  unsigned getULEB128Size(uint64_t Value);

  /// Utility function to get the size of the SLEB128-encoded value.
  unsigned getSLEB128Size(int64_t Value);

  // append methods
  void append_didl();

  void append_byte(const std::byte b);
  void append_bytes(const uint8_t *bytes, const uint32_t num_bytes);
  void append(const VecBytes &other);

  void append_sleb128(const __int128_t &v);
  template <class T> void append_sleb128(T) = delete;

  void append_uleb128(const bool &v);
  void append_uleb128(const __uint128_t &v);
  template <class T> void append_uleb128(T) = delete;

  template <typename T>
    requires MyFixedWidthInts<T>
  void append_int_fixed_width(const T &v);

  template <typename T>
    requires MyFloats<T>
  void append_float_ieee754(const T &v);

  // parse methods
  void trap_if_vec_does_not_start_with_DIDL();

  bool parse_uleb128(__uint128_t &offset, __uint128_t &v, __uint128_t &numbytes,
                     std::string &parse_error);
  template <class T>
  bool parse_uleb128(__uint128_t &, T, __uint128_t &numbytes,
                     std::string &parse_error) = delete;

  bool parse_sleb128(__uint128_t &offset, __int128_t &v, __uint128_t &numbytes,
                     std::string &parse_error);
  template <class T>
  bool parse_sleb128(__uint128_t &, T, __uint128_t &numbytes,
                     std::string &parse_error) = delete;

  template <typename T>
    requires MyFixedWidthInts<T>
  bool parse_int_fixed_width(__uint128_t &offset, T &v,
                             std::string &parse_error);

  template <typename T>
    requires MyFloats<T>
  bool parse_float_ieee754(__uint128_t &offset, T &v, std::string &parse_error);

  bool parse_bytes(__uint128_t &offset, std::vector<std::byte> &v,
                   __uint128_t &n, __uint128_t &numbytes,
                   std::string &parse_error);
  bool parse_bytes(__uint128_t &offset, std::vector<uint8_t> &v, __uint128_t &n,
                   __uint128_t &numbytes, std::string &parse_error);

  // store methods
  void store(const uint8_t *bytes, const uint32_t num_bytes);
  void store_hex_string(const std::string candid_in);
  void store_hex_tokens(std::vector<std::string> hex_tokens);

  // helper methods
  std::string as_hex_string();
  void debug_print_as_hex_string(__uint128_t B_offset_start,
                                 __uint128_t B_offset_end);
  void debug_print_as_hex_string(std::string hex_string,
                                 __uint128_t B_offset_start,
                                 __uint128_t B_offset_end);
  void debug_print();
  const std::vector<std::byte> &vec() const { return m_vec; };
  const std::vector<uint8_t> &vec_uint8_t() const { return m_vec_uint8_t; };
  int size() const { return m_vec.size(); }
  void clear();
  bool is_little_endian();
  bool is_big_endian();
  bool is_mixed_endian();
  bool is_float_ieee754();

  // Provide an equal operator to enable comparisons
  friend bool operator==(VecBytes const &lhs, VecBytes const &rhs) {
    return lhs.m_vec == rhs.m_vec;
  }

private:
  void check_endian();
  void trap(const std::string &msg);

  std::vector<std::byte> m_vec;
  std::vector<uint8_t> m_vec_uint8_t;
  std::string m_endian_type = "";
};