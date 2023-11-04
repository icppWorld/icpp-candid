// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_float64.h"
#include "vec_bytes.h"

class CandidTypeVecFloat64 : public CandidTypeBase<CandidTypeVecFloat64> {
public:
  // Constructors
  CandidTypeVecFloat64();
  CandidTypeVecFloat64(const std::vector<double> v);

  CandidTypeVecFloat64(std::vector<double> *p_v);

  // Destructor
  ~CandidTypeVecFloat64();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<double> get_v() { return m_v; }
  std::vector<double> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<double> *v) { m_pv = v; }
  void set_v(const std::vector<double> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<double> m_v;
  std::vector<double> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};