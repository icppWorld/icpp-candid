// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_float32.h"
#include "vec_bytes.h"

class CandidTypeVecFloat32 : public CandidTypeBase<CandidTypeVecFloat32> {
public:
  // Constructors
  CandidTypeVecFloat32();
  CandidTypeVecFloat32(const std::vector<float> v);

  CandidTypeVecFloat32(std::vector<float> *p_v);

  // Destructor
  ~CandidTypeVecFloat32();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<float> get_v() { return m_v; }
  std::vector<float> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<float> *v) { m_pv = v; }
  void set_v(const std::vector<float> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<float> m_v;
  std::vector<float> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};