// The class for the Candid Type: record

#pragma once

#include <cstring>
#include <memory>

#include "vec_bytes.h"

class CandidTypeRecord : public CandidTypeBase<CandidTypeRecord> {
public:
  // Constructors
  // docs start: demo_candid_type_record
  CandidTypeRecord();
  void append(uint32_t field_id, CandidType field);
  void append(std::string field_name, CandidType field);
  void append(CandidType field); // docs end: demo_candid_type_record

  // Destructor
  ~CandidTypeRecord();

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);
  bool decode_M(VecBytes B, __uint128_t &offset, std::string &parse_error);
  std::vector<std::string> get_v() { return m_field_names; }
  std::vector<std::string> *get_pv() { return &m_field_names; }
  std::vector<uint32_t> get_field_ids() { return m_field_ids; }
  std::vector<std::string> get_field_names() { return m_field_names; }

  void check_type_table(const CandidTypeRecord *p_from_wire);

protected:
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  // Declaration is moved to candid_type_base.h
  // So it can be accessed during (de-)serialization
  // std::vector<uint32_t> m_field_ids; // id | hash
  // std::vector<std::string> m_field_names;
  // std::vector<int> m_field_datatypes;
  // std::vector<std::shared_ptr<CandidTypeRoot>> m_fields_ptrs;

  // To help with decoding checks
  std::vector<int> m_field_datatypes_wire;
};