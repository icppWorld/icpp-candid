// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_bool.h"
#include "vec_bytes.h"

class CandidTypeVecBool : public CandidTypeBase<CandidTypeVecBool> {
public:
  // Constructors
  CandidTypeVecBool();
  CandidTypeVecBool(const std::vector<bool> v);

  CandidTypeVecBool(std::vector<bool> *p_v);

  // Destructor
  ~CandidTypeVecBool();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<bool> get_v() { return m_v; }
  std::vector<bool> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<bool> *v) { m_pv = v; }
  void set_v(const std::vector<bool> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<bool> m_v;
  std::vector<bool> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};