// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_float32.h"
#include "vec_bytes.h"

class CandidTypeOptFloat32 : public CandidTypeBase<CandidTypeOptFloat32> {
public:
  // Constructors
  CandidTypeOptFloat32();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptFloat32(std::optional<float> *v);
  CandidTypeOptFloat32(const std::optional<float> v); // docs end: demo_candid_type_opt
  // clang-format on

  // Destructor
  ~CandidTypeOptFloat32();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::optional<float> get_v() { return m_v; }
  std::optional<float> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(std::optional<float> *v) { m_pv = v; }
  void set_v(const std::optional<float> &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  std::optional<float> m_v;
  std::optional<float> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};