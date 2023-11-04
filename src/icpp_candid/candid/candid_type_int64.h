// The class for the Primitive Candid Type: int64

#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeInt64 : public CandidTypeBase<CandidTypeInt64> {
public:
  // Constructors
  CandidTypeInt64();
  // docs start: demo_candid_type_int64
  CandidTypeInt64(int64_t *v);
  CandidTypeInt64(const int64_t v); // docs end: demo_candid_type_int64

  // Destructor
  ~CandidTypeInt64();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  int64_t get_v() { return m_v; }
  int64_t *get_pv() { return m_pv; }

protected:
  void set_pv(int64_t *v);
  void initialize(const int64_t &v);
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();

  int64_t m_v;
  int64_t *m_pv{nullptr};
};