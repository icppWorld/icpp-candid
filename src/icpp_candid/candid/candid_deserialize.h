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

  CandidArgs get_A() { return m_A; }
  std::string get_hex_string_wire() { return m_hex_string_wire; }
  VecBytes get_B() { return m_B; }
  std::vector<CandidTypeTable> get_typetables_wire() {
    return m_typetables_wire;
  }

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

  int get_opcode_from_datatype_on_wire(int datatype);

private:
  void deserialize();

  void select_decoder_or_trap(size_t i, size_t j,
                              std::shared_ptr<CandidTypeRoot> &decoder,
                              std::string &decoder_name);

  std::shared_ptr<CandidTypeRoot>
  build_decoder_wire_for_additional_opt_arg(int j);

  // The vector with placeholders for the expected arguments
  CandidArgs m_A;

  // The deserialized type tables found on wire
  std::vector<CandidTypeTable> m_typetables_wire;

  // The deserialized args found on wire
  std::vector<int> m_args_datatypes_wire;
  std::vector<int> m_args_opcodes_wire;
  std::vector<int> m_args_content_datatypes_wire;
  std::vector<int> m_args_content_opcodes_wire;
  std::vector<__uint128_t> m_args_datatypes_offset_start_wire;
  std::vector<__uint128_t> m_args_datatypes_offset_end_wire;

  // The byte stream to be deserialized
  std::string m_hex_string_wire;
  VecBytes m_B;
};