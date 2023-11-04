// The class for the Primitive Candid Type: null

#pragma once

#include "candid_type_base.h"

class CandidTypeNull : public CandidTypeBase<CandidTypeNull> {
public:
  // Constructors
  CandidTypeNull();

  // Destructor
  ~CandidTypeNull();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error) {
    return false; // Nothing to decode for Null
  }
  std::string get_v() { return m_v; }
  std::string *get_pv() { return m_pv; }

protected:
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();
  std::string m_v{"NULL_VALUE"};
  std::string *m_pv{&m_v};
};