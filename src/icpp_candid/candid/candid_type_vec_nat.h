// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_nat.h"
#include "vec_bytes.h"

class CandidTypeVecNat : public CandidTypeBase<CandidTypeVecNat> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecNat();
  CandidTypeVecNat(const std::vector<__uint128_t> v);

  CandidTypeVecNat(std::vector<__uint128_t> *p_v);

  // Destructor
  ~CandidTypeVecNat();

  std::vector<__uint128_t> get_v() { return m_v; }
  std::vector<__uint128_t> *get_pv() { return m_pv; }

protected:
  void set_pv(std::vector<__uint128_t> *v) { m_pv = v; }
  void set_v(const std::vector<__uint128_t> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<__uint128_t> m_v;
  std::vector<__uint128_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};