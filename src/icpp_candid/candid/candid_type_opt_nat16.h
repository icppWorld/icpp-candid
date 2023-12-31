// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_nat16.h"
#include "vec_bytes.h"

class CandidTypeOptNat16 : public CandidTypeBase<CandidTypeOptNat16> {
public:
  // Constructors
  CandidTypeOptNat16();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptNat16(std::optional<uint16_t> *v);
  CandidTypeOptNat16(const std::optional<uint16_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptNat16();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<uint16_t> get_v() { return m_v; }
  std::optional<uint16_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<uint16_t> *v) { m_pv = v; }
  void set_v(const std::optional<uint16_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<uint16_t> m_v;
  std::optional<uint16_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};