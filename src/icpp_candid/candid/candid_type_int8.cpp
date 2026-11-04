// The class for the Primitive Candid Type: int8

#include "candid_type.h"
#include "candid_type_int8.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeInt8::CandidTypeInt8() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeInt8::CandidTypeInt8(int8_t *p_v) {

  set_pv(p_v);

  const int8_t v = const_cast<int8_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeInt8::CandidTypeInt8(const int8_t v) { initialize(v); }

CandidTypeInt8::~CandidTypeInt8() {}

// Initialize things
void CandidTypeInt8::initialize(const int8_t &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeInt8::set_pv(int8_t *v) { m_pv = v; }

void CandidTypeInt8::set_datatype() {
  m_datatype_opcode = CandidOpcode().Int8;
  m_datatype_hex = OpcodeHex().Int8;
  m_datatype_textual = OpcodeTextual().Int8;
}

void CandidTypeInt8::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeInt8::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : int<N>)   = i<N>(signed_N^-1(i))    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}
