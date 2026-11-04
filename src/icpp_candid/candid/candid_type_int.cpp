// The class for the Primitive Candid Type: int

#include "candid_type.h"
#include "candid_type_int.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeInt::CandidTypeInt() { initialize(0); }

// This constructor allows for setting the value during Deserialization
CandidTypeInt::CandidTypeInt(__int128_t *p_v) {
  set_pv(p_v);

  const __int128_t v = const_cast<__int128_t &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeInt::CandidTypeInt(const __int128_t &v) { initialize(v); }

CandidTypeInt::~CandidTypeInt() {}

// Initialize things
void CandidTypeInt::initialize(const __int128_t &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeInt::set_pv(__int128_t *v) { m_pv = v; }

void CandidTypeInt::set_datatype() {
  m_datatype_opcode = CandidOpcode().Int;
  m_datatype_hex = OpcodeHex().Int;
  m_datatype_textual = OpcodeTextual().Int;
}

void CandidTypeInt::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeInt::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(i : int)      = sleb128(i)
  m_M.append_sleb128(m_v);
}
