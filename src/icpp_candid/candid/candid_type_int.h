// The class for the Primitive Candid Type: int

#pragma once

#include "candid_bind.h"
#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeInt : public CandidTypeBase<CandidTypeInt> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeInt();
  // docs start: demo_candid_type_int
  CandidTypeInt(__int128_t *v);
  CandidTypeInt(const __int128_t &v); // docs end: demo_candid_type_int

  // Destructor
  ~CandidTypeInt();

  __int128_t get_v() { return m_v; }
  __int128_t *get_pv() { return m_pv; }

protected:
  void set_pv(__int128_t *v);
  void initialize(const __int128_t &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  __int128_t m_v;
  __int128_t *m_pv{nullptr};
};