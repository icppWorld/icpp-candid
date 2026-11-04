// The class for the Candid Type: text

#pragma once

#include "candid_bind.h"
#include <cstring>

#include "candid_type_text.h"
#include "vec_bytes.h"

class CandidTypeVecText : public CandidTypeBase<CandidTypeVecText> {
public:
  CANDID_BIND_VEC

  // Constructors
  CandidTypeVecText();
  CandidTypeVecText(const std::vector<std::string> v);

  CandidTypeVecText(std::vector<std::string> *p_v);

  // Destructor
  ~CandidTypeVecText();

  std::vector<std::string> get_v() { return m_v; }
  std::vector<std::string> *get_pv() { return m_pv; }

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