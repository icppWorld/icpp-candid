// The class for the Primitive Candid Type: nat32

#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeNat32 : public CandidTypeBase<CandidTypeNat32> {
public:
  // Constructors
  CandidTypeNat32();
  // docs start: demo_candid_type_nat32
  CandidTypeNat32(uint32_t *v);
  CandidTypeNat32(const uint32_t v); // docs end: demo_candid_type_nat32

  // Destructor
  ~CandidTypeNat32();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  uint32_t get_v() { return m_v; }
  uint32_t *get_pv() { return m_pv; }

protected:
  void set_pv(uint32_t *v);
  void initialize(const uint32_t &v);
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();

  uint32_t m_v;
  uint32_t *m_pv{nullptr};
};