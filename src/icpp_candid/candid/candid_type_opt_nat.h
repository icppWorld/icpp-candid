// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_nat.h"
#include "vec_bytes.h"

class CandidTypeOptNat : public CandidTypeBase<CandidTypeOptNat> {
public:
  // Constructors
  CandidTypeOptNat();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptNat(std::optional<__uint128_t> *v);
  CandidTypeOptNat(const std::optional<__uint128_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptNat();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<__uint128_t> get_v() { return m_v; }
  std::optional<__uint128_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<__uint128_t> *v) { m_pv = v; }
  void set_v(const std::optional<__uint128_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<__uint128_t> m_v;
  std::optional<__uint128_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};