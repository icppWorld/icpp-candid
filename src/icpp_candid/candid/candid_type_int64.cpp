// The class for the Primitive Candid Type: int64

#include "candid_type.h"
#include "candid_type_int64.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeInt64::CandidTypeInt64() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeInt64::CandidTypeInt64(int64_t *p_v) {

  set_pv(p_v);

  const int64_t v = const_cast<int64_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeInt64::CandidTypeInt64(const int64_t v) { initialize(v); }

CandidTypeInt64::~CandidTypeInt64() {}

// Initialize things
void CandidTypeInt64::initialize(const int64_t &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeInt64::set_pv(int64_t *v) { m_pv = v; }

void CandidTypeInt64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Int64;
  m_datatype_hex = OpcodeHex().Int64;
  m_datatype_textual = OpcodeTextual().Int64;
}

void CandidTypeInt64::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeInt64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(n : int<N>)   = i<N>(signed_N^-1(i))    (Litte Endian)
  m_M.append_int_fixed_width(m_v);
}
