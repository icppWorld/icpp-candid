// The class for the Primitive Candid Type: int32

#include "candid_type.h"
#include "candid_type_int32.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeInt32::CandidTypeInt32() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeInt32::CandidTypeInt32(int32_t *p_v) {

  set_pv(p_v);

  const int32_t v = const_cast<int32_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeInt32::CandidTypeInt32(const int32_t v) { initialize(v); }

CandidTypeInt32::~CandidTypeInt32() {}

// Initialize things
void CandidTypeInt32::initialize(const int32_t &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeInt32::set_pv(int32_t *v) { m_pv = v; }

void CandidTypeInt32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Int32;
  m_datatype_hex = OpcodeHex().Int32;
  m_datatype_textual = OpcodeTextual().Int32;
}

void CandidTypeInt32::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeInt32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : int<N>)   = i<N>(signed_N^-1(i))    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}
