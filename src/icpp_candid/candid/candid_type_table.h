// The candid type table of a Candid variable
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include "candid_constants.h"
#include <memory>

class CandidTypeTable {
public:
  CandidTypeTable();
  CandidTypeTable(const VecBytes &B, __uint128_t &B_offset);
  ~CandidTypeTable();

  VecBytes get_B() { return m_B; }
  int get_datatype() { return m_datatype; }
  int get_opcode() { return m_opcode; }
  int get_content_datatype() { return m_content_datatype; }
  int get_content_opcode() { return m_content_opcode; }

  std::shared_ptr<CandidTypeRoot> get_p_wire() { return m_p_wire; }

  void set_opcode(int opcode) { m_opcode = opcode; }
  void finish_vec_and_opt(int content_opcode,
                          CandidTypeTable *p_content_type_table);

private:
  void deserialize(__uint128_t &B_offset);

  int m_datatype{CANDID_UNDEF}; // could be a type table index
  int m_opcode{CANDID_UNDEF};   // always < 0, walk the type tables if needed

  // only for Vec & Opt
  int m_content_datatype{CANDID_UNDEF};
  int m_content_opcode{CANDID_UNDEF};

  // A dummy CandidType, to use it's encode & decode methods
  std::shared_ptr<CandidTypeRoot> m_p_wire{nullptr};

  // The byte stream to be deserialized
  __uint128_t m_B_offset;
  VecBytes m_B;
  std::string m_hex_string_wire;
};