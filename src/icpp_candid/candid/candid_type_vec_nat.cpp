// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_nat.h"

#include <cassert>

CandidTypeVecNat::CandidTypeVecNat() : CandidTypeBase() {
  std::vector<__uint128_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecNat::CandidTypeVecNat(std::vector<__uint128_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<__uint128_t> v =
      const_cast<std::vector<__uint128_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecNat::CandidTypeVecNat(const std::vector<__uint128_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecNat::~CandidTypeVecNat() {}

void CandidTypeVecNat::set_content_type() {
  m_content_opcode = CandidOpcode().Nat;
  m_content_hex = OpcodeHex().Nat;
  m_content_textual = OpcodeTextual().Nat;
}

void CandidTypeVecNat::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // Nat:             - M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  for (__uint128_t const &c : m_v) {
    m_M.append_uleb128(c);
  }
}

// ---------
// Initialize things
void CandidTypeVecNat::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecNat::set_datatype() {
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
void CandidTypeVecNat::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
