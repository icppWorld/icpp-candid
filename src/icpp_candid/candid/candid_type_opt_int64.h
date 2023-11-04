// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_int64.h"
#include "vec_bytes.h"

class CandidTypeOptInt64 : public CandidTypeBase<CandidTypeOptInt64> {
public:
  // Constructors
  CandidTypeOptInt64();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt64(std::optional<int64_t> *v);
  CandidTypeOptInt64(const std::optional<int64_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptInt64();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<int64_t> get_v() { return m_v; }
  std::optional<int64_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<int64_t> *v) { m_pv = v; }
  void set_v(const std::optional<int64_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int64_t> m_v;
  std::optional<int64_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};