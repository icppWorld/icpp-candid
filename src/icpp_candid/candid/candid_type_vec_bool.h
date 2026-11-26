// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_bool.h"
#include "vec_bytes.h"

class CandidTypeVecBool : public CandidTypeBase<CandidTypeVecBool> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecBool();
  CandidTypeVecBool(std::vector<bool> v);

  CandidTypeVecBool(std::vector<bool> *p_v);

  std::vector<bool> get_v() { return m_v; }
  std::vector<bool> *get_pv() { return m_pv; }

protected:
  // Reads an element of m_v without copying the vector
  friend struct GetCandidTypeVecIndexVisitor;

  void set_pv(std::vector<bool> *v) { m_pv = v; }
  void set_v(std::vector<bool> v) {
    m_v_size = v.size();
    m_v = std::move(v);
  }
  void set_content_type();
  void encode_M();

  std::vector<bool> m_v;
  std::vector<bool> *m_pv{nullptr};

  void initialize();
  void set_datatype();
};