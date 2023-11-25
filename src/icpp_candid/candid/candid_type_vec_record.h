// The class for the Candid Type: text

#pragma once

#include <cstring>

#include "candid_args.h"
#include "candid_type_record.h"
#include "vec_bytes.h"

class CandidTypeVecRecord : public CandidTypeBase<CandidTypeVecRecord> {
public:
  // Constructors
  CandidTypeVecRecord();
  CandidTypeVecRecord(const CandidTypeRecord v);

  CandidTypeVecRecord(CandidTypeRecord *p_v);

  // Internal: During deserialization for an expected argument
  CandidTypeVecRecord(CandidTypeRoot *v);

  // Internal: During deserialization for an additional wire argument (dummy !)
  CandidTypeVecRecord(std::shared_ptr<CandidTypeRoot> p_wire_record);

  // Destructor
  ~CandidTypeVecRecord();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);

  CandidTypeRecord get_v() { return m_v; }
  CandidTypeRoot *get_pv() { return m_pv; }
  std::shared_ptr<CandidTypeRoot> get_pvs() { return m_pvs; }
  std::shared_ptr<CandidTypeRecord> get_pr() { return m_pr; }

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

  // a shared pointer to a dummy record
  std::shared_ptr<CandidTypeRecord> m_pr{nullptr};
  void create_dummy_record_encode();

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};