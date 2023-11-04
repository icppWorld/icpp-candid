// The class for the Primitive Candid Type: int8

#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeInt8 : public CandidTypeBase<CandidTypeInt8> {
public:
  // Constructors
  CandidTypeInt8();
  // docs start: demo_candid_type_int8
  CandidTypeInt8(int8_t *v);
  CandidTypeInt8(const int8_t v); // docs end: demo_candid_type_int8

  // Destructor
  ~CandidTypeInt8();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  int8_t get_v() { return m_v; }
  int8_t *get_pv() { return m_pv; }

protected:
  void set_pv(int8_t *v);
  void initialize(const int8_t &v);
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();

  int8_t m_v;
  int8_t *m_pv{nullptr};
};