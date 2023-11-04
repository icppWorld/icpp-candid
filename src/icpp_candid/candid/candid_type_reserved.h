// The class for the Primitive Candid Type: reserved
#pragma once

#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeReserved : public CandidTypeBase<CandidTypeReserved> {
public:
  // Constructors
  CandidTypeReserved();

  // Destructor
  ~CandidTypeReserved();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error) {
    return false; // type table for Primitives is empty
  }
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::string get_v() { return m_v; }
  std::string *get_pv() { return m_pv; }

protected:
  void set_datatype();
  void encode_T() { m_T.clear(); } // type table for Primitives is empty
  void encode_I();
  void encode_M();
  std::string m_v{"RESERVED_VALUE"};
  std::string *m_pv{&m_v};
};