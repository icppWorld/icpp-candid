// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_float32.h"

#include <cassert>

CandidTypeVecFloat32::CandidTypeVecFloat32() : CandidTypeBase() {
  std::vector<float> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecFloat32::CandidTypeVecFloat32(std::vector<float> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  set_v(*p_v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecFloat32::CandidTypeVecFloat32(std::vector<float> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeVecFloat32::set_content_type() {
  m_content_opcode = CandidOpcode().Float32;
  m_content_hex = OpcodeHex().Float32;
  m_content_textual = opcode_textual().Float32;
}

void CandidTypeVecFloat32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(z : float<N>) = f<N>(z)
  m_M.append_vec_fixed_width(m_v);
}

// ---------
// Initialize things
void CandidTypeVecFloat32::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecFloat32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecFloat32::encode_I() {}
