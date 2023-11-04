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
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);
  std::vector<std::string> get_v() { return m_field_names; }
  std::vector<std::string> *get_pv() { return &m_field_names; }

  void set_fields_wire(std::shared_ptr<CandidTypeRoot> p_from_wire);
  void finish_decode_T(CandidDeserialize &de);

protected:
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  void select_decoder_or_trap(CandidDeserialize &de, size_t i, size_t j,
                              std::shared_ptr<CandidTypeRoot> &decoder,
                              std::string &decoder_name);

  std::shared_ptr<CandidTypeRoot>
  build_decoder_wire_for_additional_opt_field(CandidDeserialize &de, int j);
};