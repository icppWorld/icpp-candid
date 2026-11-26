// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_float64.h"

#include <cassert>

CandidTypeVecFloat64::CandidTypeVecFloat64() : CandidTypeBase() {
  std::vector<double> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecFloat64::CandidTypeVecFloat64(std::vector<double> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  set_v(*p_v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecFloat64::CandidTypeVecFloat64(std::vector<double> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeVecFloat64::set_content_type() {
  m_content_opcode = CandidOpcode().Float64;
  m_content_hex = OpcodeHex().Float64;
  m_content_textual = opcode_textual().Float64;
}

void CandidTypeVecFloat64::encode_M() {
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
void CandidTypeVecFloat64::initialize() {
  set_datatype();
  set_content_type();
  encode_M();
}

void CandidTypeVecFloat64::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}
