// The class for the Candid Type: text

#pragma once

#include <cstring>

#include "candid_args.h"
#include "candid_type_record.h"
#include "candid_type_vec_base.h"
#include "vec_bytes.h"

class CandidTypeVecRecord : public CandidTypeVecBase {
public:
  // Constructors
  CandidTypeVecRecord();
  CandidTypeVecRecord(const CandidTypeRecord v);

  CandidTypeVecRecord(CandidTypeRecord *p_v);

  // Destructor
  ~CandidTypeVecRecord();

  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  CandidTypeRecord get_v() { return m_v; }

protected:
  void set_pv(CandidTypeRecord *v) { m_pv = v; }
  void set_v(const CandidTypeRecord &v) { m_v = v; }
  void set_content_type();
  void encode_M();

  CandidTypeRecord m_v;
  CandidTypeRecord *m_pv{nullptr};
};