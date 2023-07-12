// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid_type.h"
#include "candid_type_all_includes.h"

class CandidDeserialize {
public:
  CandidDeserialize();
  CandidDeserialize(const VecBytes &B, std::vector<CandidType> A);
  CandidDeserialize(const std::string hex_string, std::vector<CandidType> A);
  ~CandidDeserialize();

  std::vector<CandidType> get_A() { return m_A; }
  VecBytes get_B() { return m_B; }

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

private:
  void deserialize();

  // The vector with placeholders for the expected arguments of the byte stream coming in
  std::vector<CandidType> m_A;

  // The deserialized type tables
  std::vector<CandidTypeTable> m_typetables;

  // The deserialized argument list
  std::vector<int> m_args_datatypes;
  std::vector<__uint128_t> m_args_datatypes_offset_start;
  std::vector<__uint128_t> m_args_datatypes_offset_end;

  // The byte stream to be deserialized
  std::string m_hex_string;
  VecBytes m_B;
};