// The class for the Candid Type: Record

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_deserialize.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_type_record.h"

#include "icpp_hooks.h"

#include <algorithm>
#include <cassert>
#include <typeinfo>
#include <utility>

#include "candid_type_of.h"

CandidTypeRecord::CandidTypeRecord() : CandidTypeBase() {

  set_datatype();
  encode_M();
}

void CandidTypeRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Record;
  m_datatype_hex = OpcodeHex().Record;
  m_datatype_textual = opcode_textual().Record;
}

// Tuple notation with uint32_t field_id
void CandidTypeRecord::append(uint32_t field_id, CandidType field) {
  _append(field_id, std::string(), std::move(field));
}

// Record notation with string name -> hash name into uint32_t field_id
void CandidTypeRecord::append(std::string field_name, CandidType field) {
  uint32_t field_id = idl_hash(field_name);
  _append(field_id, std::move(field_name), std::move(field));
}

void CandidTypeRecord::append_optional(std::string field_name, CandidType field,
                                       bool *p_on_wire) {
  uint32_t field_id = idl_hash(field_name);
  _append(field_id, std::move(field_name), std::move(field));
  m_optional_fields[field_id] = p_on_wire;
}

// Tuple notation without field_id -> generate sequential uint32_t field_id
void CandidTypeRecord::append(CandidType field) {
  if (m_field_ptrs.size() == 0) {
    uint32_t field_id = 0;
    append(field_id, std::move(field));
  } else if (in_range<std::uint32_t>(m_field_ids.back() + 1)) {
    uint32_t field_id = m_field_ids.back() + 1;
    append(field_id, std::move(field));
  } else {
    std::string msg;
    msg.append("ERROR: field_id outside range of uint32_t\n");
    msg.append("       id of previous field in record: " +
               std::to_string(m_field_ids.back()) + "\n");
    msg.append(
        "       because no field_id was specified for current field, we want to increase it by 1");
    ICPP_HOOKS::trap(msg);
  }
}

void CandidTypeRecord::_append(uint32_t field_id, std::string field_name,
                               CandidType &&field) {
  insert_field(field_id, std::move(field_name),
               candid_type_of::root_ptr(std::move(field)), "record");
  // Encoded once, when the value is read, not after every field
  m_M_stale = true;
}

bool CandidTypeRecord::certainly_same_type(const CandidTypeRoot &other) const {
  if (typeid(other) != typeid(CandidTypeRecord)) return false;
  const auto &o = static_cast<const CandidTypeRecord &>(other);
  if (!m_optional_fields.empty() || !o.m_optional_fields.empty() ||
      m_field_ids != o.m_field_ids)
    return false;
  for (size_t i = 0; i < m_field_ptrs.size(); ++i)
    if (!m_field_ptrs[i]->certainly_same_type(*o.m_field_ptrs[i])) return false;
  return true;
}

// (re-)build the value encoding
void CandidTypeRecord::encode_M() {
  m_M.clear();
  m_has_M = true;

  // The fields are never null: insert_field adds them
  for (const auto &p_field : m_field_ptrs) {
    if (!p_field->has_M()) m_has_M = false;
    m_M.append(p_field->get_M());
  }
}

// An expected field that may be absent on the wire
int CandidTypeRecord::graph_type(CandidTypeGraph &g) {
  std::vector<CandidGraphField> fields;
  for (size_t i = 0; i < m_field_ids.size(); ++i) {
    CandidGraphField f;
    f.id = m_field_ids[i];
    f.type = m_field_ptrs[i]->graph_type(g);
    f.optional = m_optional_fields.count(f.id) > 0;
    fields.push_back(f);
  }
  return g.record(std::move(fields));
}

void CandidTypeRecord::bind(const CandidValue &v) {
  for (size_t i = 0; i < m_field_ids.size(); ++i) {
    const CandidValue *f = v.field(m_field_ids[i]);
    if (!f) {
      ICPP_HOOKS::trap("ERROR: CandidTypeRecord - no value for field " +
                       m_field_names[i]);
    }
    const CandidValue &fv = *f;
    auto opt = m_optional_fields.find(m_field_ids[i]);
    if (opt != m_optional_fields.end() && opt->second) {
      *opt->second = !fv.absent;
    }
    // A field added with append_optional, not on the wire, is left as is
    if (!fv.absent) m_field_ptrs[i]->bind(fv);
  }
}
