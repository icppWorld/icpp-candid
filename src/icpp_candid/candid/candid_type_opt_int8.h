// The class for the Candid Type: opt

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_int8.h"
#include "vec_bytes.h"

class CandidTypeOptInt8 : public CandidTypeBase<CandidTypeOptInt8> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeOptInt8();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt8(std::optional<int8_t> *v);
  CandidTypeOptInt8(std::optional<int8_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  std::optional<int8_t> get_v() { return m_v; }
  std::optional<int8_t> *get_pv() { return m_pv; }

protected:
  void set_pv(std::optional<int8_t> *v) { m_pv = v; }
  void set_v(std::optional<int8_t> v) { m_v = std::move(v); }
  void set_content_type();
  void encode_M();

  std::optional<int8_t> m_v;
  std::optional<int8_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_I();
};