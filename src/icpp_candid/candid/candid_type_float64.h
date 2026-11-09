// The class for the Primitive Candid Type: float64

#pragma once

#include "candid_bind.h"
#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeFloat64 : public CandidTypeBase<CandidTypeFloat64> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeFloat64();
  // docs start: demo_candid_type_float64
  CandidTypeFloat64(double *v);
  CandidTypeFloat64(const double v); // docs end: demo_candid_type_float64

  // Destructor
  ~CandidTypeFloat64();

  double get_v() { return m_v; }
  double *get_pv() { return m_pv; }

protected:
  void set_pv(double *v);
  void initialize(const double &v);
  void set_datatype();
  void encode_I();
  void encode_M();

  double m_v;
  double *m_pv{nullptr};
};