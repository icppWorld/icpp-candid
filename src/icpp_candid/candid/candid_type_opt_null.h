// The class for the Candid Type: opt

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_null.h"
#include "vec_bytes.h"

class CandidTypeOptNull : public CandidTypeBase<CandidTypeOptNull> {
public:
  CANDID_BIND_VALUE

  // Constructors
  // CandidTypeOptNull();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptNull(); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptNull();

  std::optional<std::string> get_v() { return m_v; }
  std::optional<std::string> *get_pv() { return m_pv; }

protected:
  void set_pv(std::optional<std::string> *v) { m_pv = v; }
  void set_v(const std::optional<std::string> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<std::string> m_v;
  std::optional<std::string> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};