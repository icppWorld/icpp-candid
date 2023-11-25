// The class for the Candid Type: opt

#pragma once

#include <cstring>

#include "candid_type_record.h"
#include "vec_bytes.h"

class CandidTypeOptRecord : public CandidTypeBase<CandidTypeOptRecord> {
public:
  // Constructors
  CandidTypeOptRecord();
  // clang-format off
  // docs start: demo_candid_type_opt
  CandidTypeOptRecord(CandidTypeRecord *v, bool *has_value);
  CandidTypeOptRecord(const CandidTypeRecord v); // docs end: demo_candid_type_opt
  // clang-format on

  // Internal: During deserialization for an expected argument
  CandidTypeOptRecord(CandidTypeRoot *v, bool *has_value);

  // Internal: During deserialization for an additional wire argument (dummy !)
  CandidTypeOptRecord(std::shared_ptr<CandidTypeRoot> v);

  // Destructor
  ~CandidTypeOptRecord();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  CandidTypeRecord get_v() { return m_v; }
  CandidTypeRoot *get_pv() { return m_pv; }
  std::shared_ptr<CandidTypeRoot> get_pvs() { return m_pvs; }
  bool has_value() { return m_p_has_value; };

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(CandidTypeRoot *p_v) { m_pv = p_v; }
  void set_v(const CandidTypeRecord &v) { m_v = v; }
  void set_pvs(std::shared_ptr<CandidTypeRoot> p_vs) { m_pvs = p_vs; }
  void set_content_type();
  void encode_M();

  CandidTypeRecord m_v;
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