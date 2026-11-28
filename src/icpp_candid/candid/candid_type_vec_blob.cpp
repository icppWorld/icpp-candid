// The class for the Candid Type: vec blob (vec vec nat8)
#include <algorithm>
#include "candid_type.h"

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
CandidTypeVecBlob::CandidTypeVecBlob(std::vector<std::vector<uint8_t>> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeVecBlob::set_content_type() {
  m_content_opcode = CandidOpcode().Vec;
  m_content_hex = OpcodeHex().Vec;
  m_content_textual = opcode_textual().Vec;
}

void CandidTypeVecBlob::encode_M() {
  m_M.clear();
  m_has_M = encode_M_into(m_M);
}

bool CandidTypeVecBlob::encode_M_into(VecBytes &out) const {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N
  // with <datatype> = vec nat8: leb128(size) followed by the bytes
  out.append_uleb128(__uint128_t(m_v.size()));
  for (const std::vector<uint8_t> &blob : m_v) {
    out.append_uleb128(__uint128_t(blob.size()));
    out.append_bytes(blob.data(), blob.size());
  }
  return true;
}

// ---------
// Initialize things
void CandidTypeVecBlob::initialize() {
  set_datatype();
  set_content_type();
  // Encoded when it is read, or straight into a message (append_M_to)
  m_M_stale = true;
}

void CandidTypeVecBlob::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}

int CandidTypeVecBlob::graph_type(CandidTypeGraph &g) {
  return g.vec(g.vec(CandidTypeGraph::prim(CandidKind::Nat8)));
}
