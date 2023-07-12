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
};