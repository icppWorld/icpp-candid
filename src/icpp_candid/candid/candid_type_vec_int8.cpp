// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_int8.h"

#include <cassert>

CandidTypeVecInt8::CandidTypeVecInt8() : CandidTypeBase() {
  std::vector<int8_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecInt8::CandidTypeVecInt8(std::vector<int8_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<int8_t> v = const_cast<std::vector<int8_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecInt8::CandidTypeVecInt8(const std::vector<int8_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecInt8::~CandidTypeVecInt8() {}

void CandidTypeVecInt8::set_content_type() {
  m_content_opcode = CandidOpcode().Int8;
  m_content_hex = OpcodeHex().Int8;
  m_content_textual = OpcodeTextual().Int8;
}

void CandidTypeVecInt8::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(n : int<N>)   = i<N>(signed_N^-1(i))    (Litte Endian)
  for (int8_t const &c : m_v) {
    m_M.append_int_fixed_width(c);
  }
}

// ---------
// Initialize things
void CandidTypeVecInt8::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecInt8::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecInt8::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
