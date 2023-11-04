// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_int8.h"
#include "vec_bytes.h"

class CandidTypeOptInt8 : public CandidTypeBase<CandidTypeOptInt8> {
public:
  // Constructors
  CandidTypeOptInt8();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt8(std::optional<int8_t> *v);
  CandidTypeOptInt8(const std::optional<int8_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptInt8();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<int8_t> get_v() { return m_v; }
  std::optional<int8_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<int8_t> *v) { m_pv = v; }
  void set_v(const std::optional<int8_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int8_t> m_v;
  std::optional<int8_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};