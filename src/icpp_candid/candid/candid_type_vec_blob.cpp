// The class for the Candid Type: vec blob (vec vec nat8)
#include <algorithm>
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_blob.h"

#include <cassert>

CandidTypeVecBlob::CandidTypeVecBlob() : CandidTypeBase() {
  std::vector<std::vector<uint8_t>> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecBlob::CandidTypeVecBlob(std::vector<std::vector<uint8_t>> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<std::vector<uint8_t>> v =
      const_cast<std::vector<std::vector<uint8_t>> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecBlob::CandidTypeVecBlob(const std::vector<std::vector<uint8_t>> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecBlob::~CandidTypeVecBlob() {}

void CandidTypeVecBlob::set_content_type() {
  m_content_opcode = CandidOpcode().Vec;
  m_content_hex = OpcodeHex().Vec;
  m_content_textual = OpcodeTextual().Vec;
}

void CandidTypeVecBlob::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N
  // with <datatype> = vec nat8: leb128(size) followed by the bytes
  m_M.append_uleb128(__uint128_t(m_v.size()));
  for (const std::vector<uint8_t> &blob : m_v) {
    m_M.append_uleb128(__uint128_t(blob.size()));
    for (uint8_t const &c : blob) {
      m_M.append_int_fixed_width(c);
    }
  }
}

// ---------
// Initialize things
void CandidTypeVecBlob::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecBlob::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding: vec <index of the type table of vec nat8>
void CandidTypeVecBlob::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // The type table of the content, vec nat8, registered in the registry
  if (!m_content) m_content = std::make_shared<CandidTypeVecNat8>();
  m_T.append_sleb128(__int128_t(m_content->get_type_table_index()));

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecBlob::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

int CandidTypeVecBlob::graph_type(CandidTypeGraph &g) {
  return g.vec(g.vec(CandidTypeGraph::prim(CandidKind::Nat8)));
}
