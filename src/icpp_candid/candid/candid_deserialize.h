// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid_args.h"
#include "candid_type_table.h"

class CandidDeserialize {
public:
  CandidDeserialize();
  CandidDeserialize(const VecBytes &B, CandidArgs A);
  CandidDeserialize(const std::string hex_string, CandidArgs A);
  ~CandidDeserialize();

  CandidArgs get_A();
  VecBytes get_B();

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

private:
  void deserialize();

  // The vector with placeholders for the expected arguments of the byte stream coming in
  CandidArgs m_A;

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