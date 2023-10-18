// Candid Assert helper class

#include "candid_assert.h"
#include "icpp_hooks.h"

CandidAssert::CandidAssert() {}
CandidAssert::~CandidAssert() {}

// Assert a candid VecBytes against a string in "hex" format (didc encode)
int CandidAssert::assert_candid(const VecBytes &B,
                                const std::string &candid_expected,
                                const bool &assert_value) {
  std::string s_hex;

  for (std::byte b : B.vec()) {
    s_hex.append(VecBytes::byte_to_hex(b, ""));
  }

  if (candid_expected == s_hex) {
    if (assert_value) return 0;
    return 1;
  } else {
    if (assert_value) {
      ICPP_HOOKS::debug_print("FAIL - assert_candid");
      ICPP_HOOKS::debug_print("Expecting:");
      ICPP_HOOKS::debug_print(candid_expected);
      ICPP_HOOKS::debug_print("Found:");
      ICPP_HOOKS::debug_print(s_hex);
      ICPP_HOOKS::debug_print("To debug, you can decode it with:");
      ICPP_HOOKS::debug_print("didc decode " + s_hex);
      return 1;
    }
    return 0;
  }
}

// Trap with parse error message
void CandidAssert::trap_with_parse_error(const __uint128_t &B_offset_start,
                                         const __uint128_t &B_offset,
                                         const std::string &to_be_parsed,
                                         const std::string &parse_error) {
  std::string msg;
  msg.append("ERROR: decoding of Candid byte stream failed.\n");
  msg.append("       trying to extract: " + to_be_parsed + "\n");
  msg.append("       parsing error:" + parse_error + "\n");
  msg.append("       byte offset:" + ICPP_HOOKS::to_string_128(B_offset_start) +
             "\n");
  ICPP_HOOKS::trap(msg);
}
