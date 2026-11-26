// The class for the Candid Type: opt

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_bool.h"
#include "vec_bytes.h"

class CandidTypeOptBool : public CandidTypeBase<CandidTypeOptBool> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeOptBool();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptBool(std::optional<bool> *v);
  CandidTypeOptBool(std::optional<bool> v); // docs end: demo_candid_type_opt
  // clang-format on

  std::optional<bool> get_v() { return m_v; }
  std::optional<bool> *get_pv() { return m_pv; }

protected:
  void set_pv(std::optional<bool> *v) { m_pv = v; }
  void set_v(std::optional<bool> v) { m_v = std::move(v); }
  void set_content_type();
  void encode_M();

  std::optional<bool> m_v;
  std::optional<bool> *m_pv{nullptr};

  void initialize();
  void set_datatype();
};