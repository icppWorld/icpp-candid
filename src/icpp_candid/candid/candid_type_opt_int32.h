// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_int32.h"
#include "vec_bytes.h"

class CandidTypeOptInt32 : public CandidTypeBase<CandidTypeOptInt32> {
public:
  // Constructors
  CandidTypeOptInt32();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptInt32(std::optional<int32_t> *p_v);
  CandidTypeOptInt32(const std::optional<int32_t> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptInt32();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<int32_t> get_v() { return m_v; }
  std::optional<int32_t> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<int32_t> *v) { m_pv = v; }
  void set_v(const std::optional<int32_t> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<int32_t> m_v;
  std::optional<int32_t> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};