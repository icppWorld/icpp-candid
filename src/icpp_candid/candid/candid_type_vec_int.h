// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_int.h"
#include "vec_bytes.h"

class CandidTypeVecInt : public CandidTypeBase<CandidTypeVecInt> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecInt();
  CandidTypeVecInt(std::vector<__int128_t> v);

  CandidTypeVecInt(std::vector<__int128_t> *p_v);

  std::vector<__int128_t> get_v() { return m_v; }
  std::vector<__int128_t> *get_pv() { return m_pv; }

protected:
  // Reads an element of m_v without copying the vector
  friend struct GetCandidTypeVecIndexVisitor;

  void set_pv(std::vector<__int128_t> *v) { m_pv = v; }
  void set_v(std::vector<__int128_t> v) {
    m_v_size = v.size();
    m_v = std::move(v);
  }
  void set_content_type();
  void encode_M();

  std::vector<__int128_t> m_v;
  std::vector<__int128_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
};