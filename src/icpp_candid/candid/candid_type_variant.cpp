// The class for the Candid Type: variant

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_deserialize.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_type_variant.h"

#include "icpp_hooks.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <utility>

#include "candid_type_of.h"

CandidTypeVariant::CandidTypeVariant() : CandidTypeBase() { initialize(); }

CandidTypeVariant::CandidTypeVariant(std::string *p_label) : CandidTypeBase() {

  set_p_label(p_label);
  initialize();
}

CandidTypeVariant::CandidTypeVariant(const std::string label)
    : CandidTypeBase() {

  m_label = label;
  initialize();
  append(label, CandidTypeNull{});
  // We only inserted a default, and we must allow the value to be overwritten
  m_label_value_set = false;
}

CandidTypeVariant::CandidTypeVariant(const std::string label, CandidType field)
    : CandidTypeBase() {

  m_label = label;
  initialize();
  append(label, std::move(field));
}

void CandidTypeVariant::initialize() {
  set_datatype();
  m_M_stale = true;
}

void CandidTypeVariant::set_datatype() {
  m_datatype_opcode = CandidOpcode().Variant;
  m_datatype_hex = OpcodeHex().Variant;
  m_datatype_textual = opcode_textual().Variant;
}

// Tuple notation with uint32_t field_id
void CandidTypeVariant::append(uint32_t field_id, CandidType field) {
  _append(field_id, std::string(), std::move(field));
}

// Variant notation with string name -> hash name into uint32_t field_id
void CandidTypeVariant::append(std::string field_name, CandidType field) {
  uint32_t field_id = idl_hash(field_name);
  _append(field_id, std::move(field_name), std::move(field));
}

// Tuple notation without field_id -> generate sequential uint32_t field_id
void CandidTypeVariant::append(CandidType field) {
  // The implicit id follows the previously appended field, not the largest id
  if (m_field_ptrs.size() == 0) {
    uint32_t field_id = 0;
    append(field_id, std::move(field));
  } else if (in_range<std::uint32_t>(m_last_appended_field_id + 1)) {
    uint32_t field_id = m_last_appended_field_id + 1;
    append(field_id, std::move(field));
  } else {
    std::string msg;
    msg.append("ERROR: field_id outside range of uint32_t\n");
    msg.append("       id of previous field in variant: " +
               std::to_string(m_last_appended_field_id) + "\n");
    msg.append(
        "       because no field_id was specified for current field, we want to increase it by 1");
    ICPP_HOOKS::trap(msg);
  }
}

void CandidTypeVariant::_append(uint32_t field_id, std::string field_name,
                                CandidType &&field) {
  auto iter = std::lower_bound(begin(m_field_ids), end(m_field_ids), field_id);
  if (iter != end(m_field_ids) && *iter == field_id && field_name == m_label &&
      !m_label_value_set) {
    // Erase the entry of the label, because it was not yet set by user
    auto i = std::distance(begin(m_field_ids), iter);
    m_field_ids.erase(m_field_ids.begin() + i);
    m_field_names.erase(m_field_names.begin() + i);
    m_field_ptrs.erase(m_field_ptrs.begin() + i);
  }

  m_last_appended_field_id = field_id;
  if (field_name == m_label) m_label_value_set = true;
  insert_field(field_id, std::move(field_name),
               candid_type_of::root_ptr(std::move(field)), "variant");

  // Encoded once, when the value is read, not after every field
  m_M_stale = true;
}

// (re-)build the value encoding
void CandidTypeVariant::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(kv   : variant {<fieldtype>*}) = leb128(i) M(kv : <fieldtype>*[i])
  m_M.clear();
  m_has_M = true;

  for (size_t i = 0; i < m_field_names.size(); ++i) {
    if (m_field_names[i] == m_label) {
      // encode index of variant options - leb128(i)
      m_M.append_uleb128(__uint128_t(i));

      // encode the variant's value
      m_has_M = m_field_ptrs[i]->has_M();
      m_M.append(m_field_ptrs[i]->get_M());

      break;
    }
  }
}

int CandidTypeVariant::graph_type(CandidTypeGraph &g) {
  std::vector<CandidGraphField> fields;
  for (size_t i = 0; i < m_field_ids.size(); ++i) {
    CandidGraphField f;
    f.id = m_field_ids[i];
    f.type = m_field_ptrs[i]->graph_type(g);
    fields.push_back(f);
  }
  return g.variant(std::move(fields));
}

void CandidTypeVariant::bind(const CandidValue &v) {
  // Encoding is lazy, and it depends on m_label, which the decode changes:
  // encode the value as it is before the decode, so that decoding leaves the
  // encoding as it was, also when it was not read yet. (A record needs no
  // such step: a decode changes only its fields, which keep their own bytes.)
  refresh_M();
  uint32_t id = v.ids.at(0);
  auto it = std::find(m_field_ids.begin(), m_field_ids.end(), id);
  if (it == m_field_ids.end()) {
    ICPP_HOOKS::trap("ERROR: CandidTypeVariant - no case with id " +
                     std::to_string(id));
  }
  size_t i = size_t(it - m_field_ids.begin());
  m_label = m_field_names[i];
  m_selected_field_id = id;
  if (m_p_label) *m_p_label = m_label;
  m_field_ptrs[i]->bind(v.items.at(0));
}
