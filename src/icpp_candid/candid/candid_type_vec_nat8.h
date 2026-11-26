// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_nat8.h"
#include "vec_bytes.h"

class CandidTypeVecNat8 : public CandidTypeBase<CandidTypeVecNat8> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecNat8();
  CandidTypeVecNat8(std::vector<uint8_t> v);

  CandidTypeVecNat8(std::vector<uint8_t> *p_v);

  std::vector<uint8_t> get_v() { return m_v; }
  std::vector<uint8_t> *get_pv() { return m_pv; }

protected:
  // Reads an element of m_v without copying the vector
  friend struct GetCandidTypeVecIndexVisitor;

  void set_pv(std::vector<uint8_t> *v) { m_pv = v; }
  void set_v(std::vector<uint8_t> v) {
    m_v_size = v.size();
    m_v = std::move(v);
  }
  void set_content_type();
  void encode_M();

  std::vector<uint8_t> m_v;
  std::vector<uint8_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
};