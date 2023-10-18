// Hooks to overwrite based on the runtime
// These hooks work for:
//  - icpp build-wasm:   deployed to canister runtime
//  - icpp build-native: running natively with MockIC

#include "icpp_hooks.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits.h>
#include <string>

#include "ic0.h"

ICPP_HOOKS::ICPP_HOOKS() : ICPP_HOOKS(false) {}

ICPP_HOOKS::ICPP_HOOKS(const bool &dbug) : m_debug_print(dbug) {}

ICPP_HOOKS::~ICPP_HOOKS() {}

void ICPP_HOOKS::debug_print(const char *message) {

  // https:
  // //wiki.sei.cmu.edu/confluence/display/c/INT36-C.+Converting+a+pointer+to+integer+or+integer+to+pointer
  //
  // Use `(uintptr_t)(void *)message` instead of `(uintptr_t)(void *)message`:
  // -> Either way works in IC canister, because target=wasm32 uses 32 bit
  // pointers.
  // -> The first approach is portable, and also works for:
  //     (-) native executable with 64 bit pointers
  //     (-) ports to wasm64 with 64 bit pointers

  ic0_debug_print((uintptr_t)(void *)message, (uint32_t)strlen(message));
}

void ICPP_HOOKS::debug_print(const std::string &s) {
  ICPP_HOOKS::debug_print(s.c_str());
}

void ICPP_HOOKS::trap(const char *message) {
  ic0_trap((uintptr_t)(void *)message, (uint32_t)strlen(message));
}

void ICPP_HOOKS::trap(const std::string &s) { ICPP_HOOKS::trap(s.c_str()); }

// TODO: MOVE THIS TO A HELPER METHOD, OUT OF THE HOOKS
std::optional<__uint128_t>
ICPP_HOOKS::string_to_uint128_t(const std::string &str) {
  __uint128_t result = 0;
  for (char c : str) {
    if (c < '0' || c > '9')
      return std::
          nullopt; // return an empty optional if the string contains a non-digit character

    __uint128_t prev_result = result;
    result = result * 10 + (c - '0');

    // If the new result is smaller than the previous result, that means an overflow occurred,
    // so return an empty optional
    if (result < prev_result) return std::nullopt;
  }

  return result;
}

// TODO: MOVE THIS TO A HELPER METHOD, OUT OF THE HOOKS
std::optional<__int128_t>
ICPP_HOOKS::string_to_int128_t(const std::string &str) {
  bool isNegative = false;
  std::string str_copy = str;

  if (str_copy[0] == '-') {
    isNegative = true;
    str_copy.erase(0, 1); // remove the '-' sign
  }

  // Call the string_to_128 function for __uint128_t
  std::optional<__uint128_t> res = string_to_uint128_t(str_copy);

  // If conversion was unsuccessful, return an empty optional
  if (!res) return std::nullopt;

  if (isNegative) {
    return -static_cast<__int128_t>(
        *res); // convert the result to negative if the original string started with '-'
  } else {
    return static_cast<__int128_t>(*res);
  }
}

// TODO: MOVE THIS TO A HELPER METHOD, OUT OF THE HOOKS
// to_string for __uint128_t, which is not provided by std::to_string
std::string ICPP_HOOKS::to_string_128(__uint128_t v) {
  // https://stackoverflow.com/a/55970931/5480536
  std::string str;
  do {
    int digit = v % 10;
    str = std::to_string(digit) + str;
    v = (v - digit) / 10;
  } while (v != 0);
  return str;
}

// TODO: MOVE THIS TO A HELPER METHOD, OUT OF THE HOOKS
// to_string for __int128_t, which is not provided by std::to_string
std::string ICPP_HOOKS::to_string_128(__int128_t v) {
  std::string str;
  bool isNegative = false;
  if (v < 0) {
    isNegative = true;
    str = to_string_128(__uint128_t(-v));
  } else {
    str = to_string_128(__uint128_t(v));
  }
  if (isNegative) {
    str = "-" + str;
  }
  return str;
}