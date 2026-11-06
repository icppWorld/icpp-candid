// The class for the Primitive Candid Type: nat16

#pragma once

#include "candid_bind.h"
#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeNat16 : public CandidTypeBase<CandidTypeNat16> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeNat16();
  // docs start: demo_candid_type_nat16
  CandidTypeNat16(uint16_t *v);
  CandidTypeNat16(const uint16_t v); // docs end: demo_candid_type_nat16

  // Destructor
  ~CandidTypeNat16();

  uint16_t get_v() { return m_v; }
  uint16_t *get_pv() { return m_pv; }

protected:
  void set_pv(uint16_t *v);
  void initialize(const uint16_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  uint16_t m_v;
  uint16_t *m_pv{nullptr};
};