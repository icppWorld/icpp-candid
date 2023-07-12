// The class for the Primitive Candid Type: null

#pragma once

#include "candid_type_base.h"

class CandidTypeNull : public CandidTypeBase {
public:
  // Constructors
  CandidTypeNull();

  // Destructor
  ~CandidTypeNull();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  std::string get_v() { return "NULL_VALUE"; }

protected:
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();
};