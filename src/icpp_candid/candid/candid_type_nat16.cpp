// The class for the Primitive Candid Type: nat16

#include "candid_type.h"
#include "candid_type_nat16.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeNat16::CandidTypeNat16() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeNat16::CandidTypeNat16(uint16_t *p_v) {

  set_pv(p_v);

  const uint16_t v = const_cast<uint16_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeNat16::CandidTypeNat16(const uint16_t v) { initialize(v); }

CandidTypeNat16::~CandidTypeNat16() {}

// Initialize things
void CandidTypeNat16::initialize(const uint16_t &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeNat16::set_pv(uint16_t *v) { m_pv = v; }

void CandidTypeNat16::set_datatype() {
  m_datatype_opcode = CandidOpcode().Nat16;
  m_datatype_hex = OpcodeHex().Nat16;
  m_datatype_textual = OpcodeTextual().Nat16;
}

void CandidTypeNat16::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeNat16::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}
