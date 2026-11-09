// The class for the Primitive Candid Type: float32

#pragma once

#include "candid_bind.h"
#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeFloat32 : public CandidTypeBase<CandidTypeFloat32> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeFloat32();
  // docs start: demo_candid_type_float32
  CandidTypeFloat32(float *v);
  CandidTypeFloat32(const float v); // docs end: demo_candid_type_float32

  // Destructor
  ~CandidTypeFloat32();

  float get_v() { return m_v; }
  float *get_pv() { return m_pv; }

protected:
  void set_pv(float *v);
  void initialize(const float &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  float m_v;
  float *m_pv{nullptr};
};