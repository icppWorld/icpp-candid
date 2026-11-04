// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_nat32.h"

#include <cassert>

CandidTypeVecNat32::CandidTypeVecNat32() : CandidTypeBase() {
  std::vector<uint32_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecNat32::CandidTypeVecNat32(std::vector<uint32_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<uint32_t> v = const_cast<std::vector<uint32_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecNat32::CandidTypeVecNat32(const std::vector<uint32_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecNat32::~CandidTypeVecNat32() {}

void CandidTypeVecNat32::set_content_type() {
  m_content_opcode = CandidOpcode().Nat32;
  m_content_hex = OpcodeHex().Nat32;
  m_content_textual = OpcodeTextual().Nat32;
}

void CandidTypeVecNat32::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // Nat32:             - M(n : nat<N>)   = i<N>(n)    (Litte Endian)
  for (uint32_t const &c : m_v) {
    m_M.append_int_fixed_width(c);
  }
}

// ---------
// Initialize things
void CandidTypeVecNat32::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecNat32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding
void CandidTypeVecNat32::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecNat32::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
