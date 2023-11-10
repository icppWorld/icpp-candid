// The class for the Candid Type: variant

#pragma once

#include <cstring>
#include <memory>

#include "vec_bytes.h"

class CandidTypeVariant : public CandidTypeBase<CandidTypeVariant> {
public:
  // Constructors
  CandidTypeVariant();
  CandidTypeVariant(std::string *p_label);
  CandidTypeVariant(const std::string label);
  CandidTypeVariant(const std::string label, const CandidType field);

  // Destructor
  ~CandidTypeVariant();

  std::vector<std::string> get_v() { return m_field_names; }
  std::vector<std::string> *get_pv() { return &m_field_names; }

  void append(uint32_t field_id, CandidType field);
  void append(std::string field_name, CandidType field);
  void append(CandidType field);

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);
  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);

  void set_fields_wire(std::shared_ptr<CandidTypeRoot> p_from_wire);
  void finish_decode_T(CandidDeserialize &de);

  std::string get_label() { return m_label; }

protected:
  void set_p_label(std::string *p_label) { m_p_label = p_label; };
  void _append(uint32_t field_id, std::string field_name, CandidType field);

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  // Label & id (hash) of the field that contains the Variant's data
  std::string m_label{""};
  bool m_label_value_set{false};

  // Pointer to the label passed in by caller during deserialization
  std::string *m_p_label{nullptr};

  void select_decoder_or_trap(CandidDeserialize &de, size_t i, size_t j,
                              std::shared_ptr<CandidTypeRoot> &decoder,
                              std::string &decoder_name);

  std::shared_ptr<CandidTypeRoot>
  build_decoder_wire_for_additional_opt_field(CandidDeserialize &de, int j);
};
