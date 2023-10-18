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

  // Destructor
  ~CandidTypeVecRecord();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);

  // Returns the m_field_names of the CandidTypeRecord
  std::vector<std::string> get_v() { return m_v.get_v(); }
  std::vector<std::string> *get_pv() { return m_v.get_pv(); }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(CandidTypeRecord *v) { m_pv = v; }
  void set_v(const CandidTypeRecord &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  CandidTypeRecord m_v;
  CandidTypeRecord *m_pv{nullptr};

  CandidTypeRecord create_dummy_record();

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
};