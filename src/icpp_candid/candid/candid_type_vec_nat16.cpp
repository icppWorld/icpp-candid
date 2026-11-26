// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_nat16.h"

#include <cassert>

CandidTypeVecNat16::CandidTypeVecNat16() : CandidTypeBase() {
  std::vector<uint16_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecNat16::CandidTypeVecNat16(std::vector<uint16_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  set_v(*p_v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecNat16::CandidTypeVecNat16(std::vector<uint16_t> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeVecNat16::set_content_type() {
  m_content_opcode = CandidOpcode().Nat16;
  m_content_hex = OpcodeHex().Nat16;
  m_content_textual = opcode_textual().Nat16;
}

void CandidTypeVecNat16::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // Nat16:             - M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  m_M.append_vec_fixed_width(m_v);
}

// ---------
// Initialize things
void CandidTypeVecNat16::initialize() {
  set_datatype();
  set_content_type();
  encode_M();
}

void CandidTypeVecNat16::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}
