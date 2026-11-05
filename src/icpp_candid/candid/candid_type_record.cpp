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
#include <utility>

CandidTypeRecord::CandidTypeRecord() : CandidTypeBase() {

  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeRecord::~CandidTypeRecord() {}

void CandidTypeRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Record;
  m_datatype_hex = OpcodeHex().Record;
  m_datatype_textual = OpcodeTextual().Record;
}

// Tuple notation with uint32_t field_id
void CandidTypeRecord::append(uint32_t field_id, CandidType field) {
  std::string s = "";
  _append(field_id, s, field);
}

// Record notation with string name -> hash name into uint32_t field_id
void CandidTypeRecord::append(std::string field_name, CandidType field) {
  uint32_t field_id = idl_hash(field_name);
  _append(field_id, field_name, field);
}

void CandidTypeRecord::append_optional(std::string field_name, CandidType field,
                                       bool *p_on_wire) {
  append(field_name, field);
  m_optional_fields[idl_hash(field_name)] = p_on_wire;
}

// Tuple notation without field_id -> generate sequential uint32_t field_id
void CandidTypeRecord::append(CandidType field) {
  if (m_field_ptrs.size() == 0) {
    uint32_t field_id = 0;
    append(field_id, field);
  } else if (in_range<std::uint32_t>(m_field_ids.back() + 1)) {
    uint32_t field_id = m_field_ids.back() + 1;
    append(field_id, field);
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
                               CandidType field) {
  // Check if field with identical hash already exists
  auto iter = std::find(begin(m_field_ids), end(m_field_ids), field_id);
  if (iter != end(m_field_ids)) {
    auto i = std::distance(begin(m_field_ids), iter);
    std::string msg;
    msg.append("ERROR: record already has a field with the same field_id.\n");
    msg.append("       field id (hash): " + std::to_string(field_id) + "\n");
    msg.append("       field name 1   : " + m_field_names[i] + "\n");
    msg.append("       field name 2   : " + field_name + "\n");
    ICPP_HOOKS::trap(msg);
  }

  // Add the field
  m_field_ids.push_back(field_id);
  m_field_names.push_back(field_name);

  // During encoding, the datatype is always a negative opcode !
  // Store the negative opcode of the field's type
  int datatype =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, field);
  assert(datatype < 0);
  m_field_datatypes.push_back(datatype);
  m_field_opcodes.push_back(datatype);

  // Store a shared pointer to the CandidTypeRoot class
  m_field_ptrs.push_back(std::visit(
      [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
        // Create shared_ptr for the derived type
        auto derivedPtr = std::make_shared<std::decay_t<decltype(arg_)>>(
            std::forward<decltype(arg_)>(arg_));

        // Cast it to a shared_ptr of the base type
        return std::static_pointer_cast<CandidTypeRoot>(derivedPtr);
      },
      field));
  // This makes a copy
  // m_field_ptrs.push_back(std::visit(
  //     [](auto &&arg) -> std::shared_ptr<CandidTypeRoot> {
  //       return std::make_shared<std::decay_t<decltype(arg)>>(arg);
  //     },
  //     field));

  // For Vec & Opt fields
  m_field_content_datatypes.push_back(
      m_field_ptrs.back()->get_content_datatype());
  m_field_content_opcodes.push_back(m_field_ptrs.back()->get_content_opcode());

  sort_fields_by_id();

  encode_M();
}

// For records, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeRecord::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

// (re-)build the value encoding
void CandidTypeRecord::encode_M() {
  m_M.clear();

  for (auto p_field : m_field_ptrs) {
    // The get_M method is in the CandidTypeBase class
    if (p_field) {
      VecBytes M = p_field->get_M();
      for (std::byte b : M.vec()) {
        m_M.append_byte(b);
      }
    }
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
