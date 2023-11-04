// The class for the Primitive Candid Type: int32

#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeInt32 : public CandidTypeBase<CandidTypeInt32> {
public:
  // Constructors
  CandidTypeInt32();
  // docs start: demo_candid_type_int32
  CandidTypeInt32(int32_t *v);
  CandidTypeInt32(const int32_t v); // docs end: demo_candid_type_int32

  // Destructor
  ~CandidTypeInt32();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  int32_t get_v() { return m_v; }
  int32_t *get_pv() { return m_pv; }

protected:
  void set_pv(int32_t *v);
  void initialize(const int32_t &v);
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();

  int32_t m_v;
  int32_t *m_pv{nullptr};
};