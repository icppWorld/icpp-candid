// The class for the Primitive Candid Type: null

#pragma once

#include "candid_bind.h"
#include "candid_type_base.h"

class CandidTypeNull : public CandidTypeBase<CandidTypeNull> {
public:
  CANDID_BIND_VALUE

  // Constructors
  CandidTypeNull();

  // Destructor
  ~CandidTypeNull();

  std::string get_v() { return m_v; }
  std::string *get_pv() { return &m_v; }

protected:
  void set_datatype();
  void encode_I();
  void encode_M();
  std::string m_v{"NULL_VALUE"};
  // No caller variable: a pointer to m_v would dangle in a copy, such as
  // variant.append("a", CandidTypeNull{}), which bind() then writes through.
  std::string *m_pv{nullptr};
};