// The class for the Candid Type: vec

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_principal.h"
#include "vec_bytes.h"

class CandidTypeVecPrincipal : public CandidTypeBase<CandidTypeVecPrincipal> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecPrincipal();
  CandidTypeVecPrincipal(std::vector<std::string> v);

  CandidTypeVecPrincipal(std::vector<std::string> *p_v);

  std::vector<std::string> get_v() { return m_v; }
  std::vector<std::string> *get_pv() { return m_pv; }

protected:
  void set_pv(std::vector<std::string> *v) { m_pv = v; }
  void set_v(std::vector<std::string> v) {
    m_v_size = v.size();
    m_v = std::move(v);
  }
  void set_content_type();
  void encode_M();

  std::vector<std::string> m_v;
  std::vector<std::string> *m_pv{nullptr};

  void initialize();
  void set_datatype();
};