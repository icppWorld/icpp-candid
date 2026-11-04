// The class for the Candid Type: record

#pragma once

#include <cstring>
#include <map>
#include <memory>

#include "vec_bytes.h"

class CandidTypeRecord : public CandidTypeBase<CandidTypeRecord> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);
  void bind(const CandidValue &v);

  // Constructors
  // docs start: demo_candid_type_record
  CandidTypeRecord();
  void append(uint32_t field_id, CandidType field);
  void append(std::string field_name, CandidType field);
  void append(CandidType field); // docs end: demo_candid_type_record

  // A field that may be absent on the wire, e.g. one a service added later.
  // It is decoded when it is on the wire, else left as is; *p_on_wire, when
  // given, tells which. It encodes exactly like append.
  void append_optional(std::string field_name, CandidType field,
                       bool *p_on_wire = nullptr);

  // Destructor
  ~CandidTypeRecord();

  std::vector<std::string> get_v() { return m_field_names; }
  std::vector<std::string> *get_pv() { return &m_field_names; }

protected:
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  // The fields added with append_optional, by id (hash), with their p_on_wire
  std::map<uint32_t, bool *> m_optional_fields;
};