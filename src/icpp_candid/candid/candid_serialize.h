// Candid serialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "candid_args.h"
#include "vec_bytes.h"

class CandidSerialize {
public:
  CandidSerialize(const CandidType &a);
  CandidSerialize(const CandidArgs &A);
  CandidSerialize();

  CandidArgs get_A() { return m_A; }
  // Without a copy: a reference, or the bytes moved out of a temporary
  const VecBytes &get_B() const & { return m_B; }
  VecBytes get_B() && { return std::move(m_B); }

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

  std::string as_hex_string() { return m_B.as_hex_string(); }

private:
  void serialize();

  // The argument list to be serialized
  CandidArgs m_A;

  // The serialized byte stream
  VecBytes m_B;
};