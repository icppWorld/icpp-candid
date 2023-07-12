// The class for the Primitive Candid Type: reserved
#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeReserved : public CandidTypeBase {
public:
  // Constructors
  CandidTypeReserved();

  // Destructor
  ~CandidTypeReserved();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::string get_v() { return "RESERVED_VALUE"; }

protected:
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();
};