// The class for the Candid Type: vec blob (vec vec nat8)

#pragma once

#include "candid_bind.h"
#include <cstring>
#include <memory>

#include "candid_type_vec_nat8.h"
#include "vec_bytes.h"

class CandidTypeVecBlob : public CandidTypeBase<CandidTypeVecBlob> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);

  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecBlob();
  CandidTypeVecBlob(std::vector<std::vector<uint8_t>> v);

  CandidTypeVecBlob(std::vector<std::vector<uint8_t>> *p_v);

  std::vector<std::vector<uint8_t>> get_v() { return m_v; }
  std::vector<std::vector<uint8_t>> *get_pv() { return m_pv; }

protected:
  // Reads an element of m_v without copying the vector
  friend struct GetCandidTypeVecIndexVisitor;

  void set_pv(std::vector<std::vector<uint8_t>> *v) { m_pv = v; }
  void set_v(std::vector<std::vector<uint8_t>> v) {
    m_v_size = v.size();
    m_v = std::move(v);
  }
  void set_content_type();
  void encode_M();

  std::vector<std::vector<uint8_t>> m_v;
  std::vector<std::vector<uint8_t>> *m_pv{nullptr};

  // The type table of the content, vec nat8, which this type table refers to
  std::shared_ptr<CandidTypeVecNat8> m_content;

  void initialize();
  void set_datatype();
};
