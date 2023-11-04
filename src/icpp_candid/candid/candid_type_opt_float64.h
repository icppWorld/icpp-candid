// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_float64.h"
#include "vec_bytes.h"

class CandidTypeOptFloat64 : public CandidTypeBase<CandidTypeOptFloat64> {
public:
  // Constructors
  CandidTypeOptFloat64();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptFloat64(std::optional<double> *v);
  CandidTypeOptFloat64(const std::optional<double> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptFloat64();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<double> get_v() { return m_v; }
  std::optional<double> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<double> *v) { m_pv = v; }
  void set_v(const std::optional<double> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<double> m_v;
  std::optional<double> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};