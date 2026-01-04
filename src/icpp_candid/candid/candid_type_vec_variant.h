// The class for the Candid Type: vec variant (unit variants only)
// Unit variants are variants where all options have null/no associated data
// Example: variant { AdminQuery; AdminUpdate }

#pragma once

#include <cstring>
#include <memory>

#include "candid_type_variant.h"
#include "vec_bytes.h"

class CandidTypeVecVariant : public CandidTypeBase<CandidTypeVecVariant> {
public:
  // Constructors

  // Default constructor
  CandidTypeVecVariant();

  // Constructor for encoding: takes a template variant and vector of labels
  // The template variant defines the valid options (e.g., AdminQuery, AdminUpdate)
  // The labels vector contains the selected option for each element
  CandidTypeVecVariant(const CandidTypeVariant &v_template,
                       const std::vector<std::string> &labels);

  // Constructor for decoding: pointer to receive labels
  CandidTypeVecVariant(CandidTypeVariant *p_v_template,
                       std::vector<std::string> *p_labels);

  // Internal: During deserialization for an expected argument
  CandidTypeVecVariant(CandidTypeRoot *p_v);

  // Internal: During deserialization for an additional wire argument
  CandidTypeVecVariant(std::shared_ptr<CandidTypeRoot> p_wire_variant);

  // Destructor
  ~CandidTypeVecVariant();

  bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                std::string &parse_error);

  void push_back_value(CandidTypeRoot &value);

  CandidTypeVariant get_v() { return m_v_template; }
  CandidTypeRoot *get_pv() { return m_pv; }
  std::shared_ptr<CandidTypeRoot> get_pvs() { return m_pvs; }

  std::vector<std::string> get_labels() { return m_labels; }
  std::vector<std::string> *get_p_labels() { return m_p_labels; }

  bool decode_T(VecBytes B, __uint128_t &offset, std::string &parse_error);

protected:
  void set_pv(CandidTypeRoot *p_v) { m_pv = p_v; }
  void set_v_template(const CandidTypeVariant &v) { m_v_template = v; }
  void set_pvs(std::shared_ptr<CandidTypeRoot> p_vs) { m_pvs = p_vs; }
  void set_content_type();
  void encode_M();

  // The template variant that defines valid options
  CandidTypeVariant m_v_template;

  // The vector of selected labels
  std::vector<std::string> m_labels;
  std::vector<std::string> *m_p_labels{nullptr};

  // For decoding
  CandidTypeRoot *m_pv{nullptr};
  std::shared_ptr<CandidTypeRoot> m_pvs{nullptr};

  // For encoding: a shared pointer to a dummy variant
  std::shared_ptr<CandidTypeVariant> m_pv_dummy{nullptr};
  void create_dummy_variant_encode();

  void initialize();
  void set_datatype();
  void encode_T();
  void encode_I();

  // Helper to find label index in the template variant
  int find_label_index(const std::string &label);
  // Helper to find label name from index
  std::string find_label_name(size_t index);
};
