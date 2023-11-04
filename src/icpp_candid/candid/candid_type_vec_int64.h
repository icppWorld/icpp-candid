// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_int64.h"
#include "vec_bytes.h"

class CandidTypeVecInt64 : public CandidTypeBase<CandidTypeVecInt64> {
public:
  // Constructors
  CandidTypeVecInt64();
  CandidTypeVecInt64(const std::vector<int64_t> v);

  CandidTypeVecInt64(std::vector<int64_t> *p_v);

  // Destructor
  ~CandidTypeVecInt64();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<int64_t> get_v() { return m_v; }
  std::vector<int64_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<int64_t> *v) { m_pv = v; }
  void set_v(const std::vector<int64_t> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<int64_t> m_v;
  std::vector<int64_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};