// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_int32.h"
#include "vec_bytes.h"

class CandidTypeVecInt32 : public CandidTypeBase<CandidTypeVecInt32> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecInt32();
  CandidTypeVecInt32(const std::vector<int32_t> v);

  CandidTypeVecInt32(std::vector<int32_t> *p_v);

  // Destructor
  ~CandidTypeVecInt32();

  std::vector<int32_t> get_v() { return m_v; }
  std::vector<int32_t> *get_pv() { return m_pv; }

protected:
  void set_pv(std::vector<int32_t> *v) { m_pv = v; }
  void set_v(const std::vector<int32_t> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<int32_t> m_v;
  std::vector<int32_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_I();
};