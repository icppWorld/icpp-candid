// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type.h"
#include "candid_type_base.h"
#include "vec_bytes.h"

class CandidTypeOptVec : public CandidTypeBase<CandidTypeOptVec> {
public:
  // Constructors
  CandidTypeOptVec();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptVec(CandidTypeVecBool *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecFloat32 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecFloat64 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecInt *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecInt8 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecInt16 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecInt32 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecInt64 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecNat *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecNat8 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecNat16 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecNat32 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecNat64 *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecPrincipal *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecRecord *v, bool *has_value);
  CandidTypeOptVec(CandidTypeVecText *v, bool *has_value);
  CandidTypeOptVec(const CandidTypeVecBool v);
  CandidTypeOptVec(const CandidTypeVecFloat32 v);
  CandidTypeOptVec(const CandidTypeVecFloat64 v);
  CandidTypeOptVec(const CandidTypeVecInt v);
  CandidTypeOptVec(const CandidTypeVecInt8 v);
  CandidTypeOptVec(const CandidTypeVecInt16 v);
  CandidTypeOptVec(const CandidTypeVecInt32 v);
  CandidTypeOptVec(const CandidTypeVecInt64 v);
  CandidTypeOptVec(const CandidTypeVecNat v);
  CandidTypeOptVec(const CandidTypeVecNat8 v);
  CandidTypeOptVec(const CandidTypeVecNat16 v);
  CandidTypeOptVec(const CandidTypeVecNat32 v);
  CandidTypeOptVec(const CandidTypeVecNat64 v);
  CandidTypeOptVec(const CandidTypeVecPrincipal v);
  CandidTypeOptVec(const CandidTypeVecRecord v);
  CandidTypeOptVec(const CandidTypeVecText v);// docs end: demo_candid_type_opt
  // clang-format on

  // Internal: During deserialization for an expected argument
  CandidTypeOptVec(CandidTypeRoot *v, bool *has_value);

  // Internal: During serialization
  CandidTypeOptVec(const CandidTypeRoot v);

  // Internal: During deserialization for an additional wire argument (dummy !)
  CandidTypeOptVec(std::shared_ptr<CandidTypeRoot> v);

  // Destructor
  ~CandidTypeOptVec();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  CandidTypeRoot get_v() { return m_v; }
  CandidTypeRoot *get_pv() { return m_pv; }
  std::shared_ptr<CandidTypeRoot> get_pvs() { return m_pvs; }
  bool has_value() { return m_p_has_value; };

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(CandidTypeRoot *p_v) { m_pv = p_v; }
  void set_v(const CandidTypeRoot &v) { m_v = v; }
  void set_pvs(std::shared_ptr<CandidTypeRoot> p_vs) { m_pvs = p_vs; }
  void set_content_type();
  void encode_M();

  CandidTypeRoot m_v;
  CandidTypeRoot *m_pv{nullptr};
  std::shared_ptr<CandidTypeRoot> m_pvs{nullptr};

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();

  // During   serialization: set to true during construction
  // During deserialization: set to false, and then flipped to true once found on the wire
  bool *m_p_has_value{nullptr};
};