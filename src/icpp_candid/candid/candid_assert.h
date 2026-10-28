// Candid Assert helper class

#pragma once

#include <string>
#include <vector>

#include "vec_bytes.h"

class CandidAssert {
public:
  CandidAssert();
  ~CandidAssert();

  static int assert_candid(const VecBytes &B,
                           const std::string &candid_expected,
                           const bool &assert_value);

  static void trap_with_parse_error(const __uint128_t &B_offset_start,
                                    const __uint128_t &B_offset,
                                    const std::string &to_be_parsed,
                                    const std::string &parse_error);

  // A type index or opcode read from the wire as sleb128, as an int. Traps when
  // it does not fit, so a huge value cannot wrap around into a valid index
  static int type_index_or_trap(const __int128_t &datatype,
                                const std::string &what);
};