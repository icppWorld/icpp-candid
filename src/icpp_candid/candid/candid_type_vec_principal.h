// The class for the Candid Type: vec

#pragma once

#include <cstring>

#include "candid_type_principal.h"
#include "vec_bytes.h"

class CandidTypeVecPrincipal : public CandidTypeBase<CandidTypeVecPrincipal> {
public:
  // Constructors
  CandidTypeVecPrincipal();
  CandidTypeVecPrincipal(const std::vector<std::string> v);

  CandidTypeVecPrincipal(std::vector<std::string> *p_v);

  // Destructor
  ~CandidTypeVecPrincipal();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<std::string> get_v() { return m_v; }
  std::vector<std::string> *get_pv() { return m_pv; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

protected:
  void set_pv(std::vector<std::string> *v) { m_pv = v; }
  void set_v(const std::vector<std::string> &v) {
    m_v = v;
    m_v_size = v.size();
  }
  void set_content_type();
  void encode_M();

  std::vector<std::string> m_v;
  std::vector<std::string> *m_pv{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};