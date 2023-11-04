// The class for the Primitive Candid Type: int16

#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeInt16 : public CandidTypeBase<CandidTypeInt16> {
public:
  // Constructors
  CandidTypeInt16();
  // docs start: demo_candid_type_int16
  CandidTypeInt16(int16_t *v);
  CandidTypeInt16(const int16_t v); // docs end: demo_candid_type_int16

  // Destructor
  ~CandidTypeInt16();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  int16_t get_v() { return m_v; }
  int16_t *get_pv() { return m_pv; }

protected:
  void set_pv(int16_t *v);
  void initialize(const int16_t &v);
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();

  int16_t m_v;
  int16_t *m_pv{nullptr};
};