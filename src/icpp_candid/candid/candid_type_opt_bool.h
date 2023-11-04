// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_bool.h"
#include "vec_bytes.h"

class CandidTypeOptBool : public CandidTypeBase<CandidTypeOptBool> {
public:
  // Constructors
  CandidTypeOptBool();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptBool(std::optional<bool> *v);
  CandidTypeOptBool(const std::optional<bool> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptBool();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<bool> get_v() { return m_v; }
  std::optional<bool> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<bool> *v) { m_pv = v; }
  void set_v(const std::optional<bool> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<bool> m_v;
  std::optional<bool> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};