// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_int16.h"
#include "vec_bytes.h"

class CandidTypeVecInt16 : public CandidTypeBase<CandidTypeVecInt16> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecInt16();
  CandidTypeVecInt16(const std::vector<int16_t> v);

  CandidTypeVecInt16(std::vector<int16_t> *p_v);

  // Destructor
  ~CandidTypeVecInt16();

  std::vector<int16_t> get_v() { return m_v; }
  std::vector<int16_t> *get_pv() { return m_pv; }

protected:
  void set_pv(std::vector<int16_t> *v) { m_pv = v; }
  void set_v(const std::vector<int16_t> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<int16_t> m_v;
  std::vector<int16_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};