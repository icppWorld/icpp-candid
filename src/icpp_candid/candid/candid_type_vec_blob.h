// The class for the Candid Type: vec blob (vec vec nat8)

#pragma once

#include <cstring>
#include <memory>

#include "candid_type_vec_nat8.h"
#include "vec_bytes.h"

class CandidTypeVecBlob : public CandidTypeBase<CandidTypeVecBlob> {
public:
  // Constructors
  CandidTypeVecBlob();
  CandidTypeVecBlob(const std::vector<std::vector<uint8_t>> v);

  CandidTypeVecBlob(std::vector<std::vector<uint8_t>> *p_v);

  // Destructor
  ~CandidTypeVecBlob();

  bool decode_M(CandidDeserialize &de, VecBytes &B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<std::vector<uint8_t>> get_v() { return m_v; }
  std::vector<std::vector<uint8_t>> *get_pv() { return m_pv; }

  bool decode_T(VecBytes &B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<std::vector<uint8_t>> *v) { m_pv = v; }
  void set_v(const std::vector<std::vector<uint8_t>> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<std::vector<uint8_t>> m_v;
  std::vector<std::vector<uint8_t>> *m_pv{nullptr};

  // The type table of the content, vec nat8, which this type table refers to
  std::shared_ptr<CandidTypeVecNat8> m_content;

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};
