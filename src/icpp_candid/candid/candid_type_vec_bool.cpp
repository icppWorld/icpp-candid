// The class for the Candid Type: vec
#include "candid_type.h"


#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_bool.h"
#include "candid_type_vec_bool.h"

#include <cassert>

CandidTypeVecBool::CandidTypeVecBool() : CandidTypeBase() {
  std::vector<bool> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecBool::CandidTypeVecBool(std::vector<bool> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<bool> v = const_cast<std::vector<bool> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecBool::CandidTypeVecBool(const std::vector<bool> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecBool::~CandidTypeVecBool() {}

void CandidTypeVecBool::set_content_type() {
  m_content_opcode = CandidOpcode().Bool;
  m_content_hex = OpcodeHex().Bool;
  m_content_textual = OpcodeTextual().Bool;
}

void CandidTypeVecBool::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(b : bool)     = i8(if b then 1 else 0)
  for (bool const &c : m_v) {
    m_M.append_uleb128(c);
  }
}

// ---------
// Initialize things
void CandidTypeVecBool::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecBool::set_datatype() {
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
void CandidTypeVecBool::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
