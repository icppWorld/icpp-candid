// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_nat16.h"
#include "vec_bytes.h"

class CandidTypeVecNat16 : public CandidTypeBase<CandidTypeVecNat16> {
public:
  // Constructors
  CandidTypeVecNat16();
  CandidTypeVecNat16(const std::vector<uint16_t> v);

  CandidTypeVecNat16(std::vector<uint16_t> *p_v);

  // Destructor
  ~CandidTypeVecNat16();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<uint16_t> get_v() { return m_v; }
  std::vector<uint16_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<uint16_t> *v) { m_pv = v; }
  void set_v(const std::vector<uint16_t> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<uint16_t> m_v;
  std::vector<uint16_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};