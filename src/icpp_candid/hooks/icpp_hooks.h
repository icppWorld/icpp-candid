// Hooks to overwrite based on the runtime

#pragma once

#include <array>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>

#include "vec_bytes.h"

class ICPP_HOOKS {
public:
  ICPP_HOOKS();

  ICPP_HOOKS(const bool &debug_print);

  ~ICPP_HOOKS();

  static void debug_print(const char *msg);
  static void debug_print(const std::string &msg);

  static void trap(const char *msg);
  static void trap(const std::string &msg);

  // Convert string to __uint128_t & __int128_t
  // clang-format off
  static std::optional<__uint128_t> string_to_uint128_t(const std::string &str);
  static std::optional<__int128_t> string_to_int128_t(const std::string &str);
  // clang-format on

  // Convert __uint128_t & __int128_t to std::string
  static std::string to_string_128(__uint128_t v);
  static std::string to_string_128(__int128_t v);

private:
  bool m_debug_print{false};
  // VecBytes m_B_in;
  // VecBytes m_B_out;
  // CandidTypePrincipal m_caller;
  // void msg_reply();

  // bool m_called_from_wire{false};
  // bool m_called_to_wire{false};
};