// The candid type table of a Candid variable
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

class CandidTypeTable {
public:
  CandidTypeTable();
  CandidTypeTable(const VecBytes &B, __uint128_t &B_offset);
  ~CandidTypeTable();

  VecBytes get_B() { return m_B; }
  int get_opcode() { return m_opcode; }
  int get_content_opcode() { return m_content_opcode; }
  CandidType *get_candidType_ptr() { return &m_c; }

  void set_vec_and_opt(int content_opcode);

private:
  void deserialize(__uint128_t &B_offset);

  int m_opcode;

  // only for Vec & Opt
  int m_content_opcode;
  size_t m_content_table_index{static_cast<size_t>(-1)};

  // A dummy CandidType, to use it's encode & decode methods
  CandidType m_c;

  // The byte stream to be deserialized
  __uint128_t m_B_offset;
  VecBytes m_B;
};