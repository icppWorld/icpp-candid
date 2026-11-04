// The class for the Candid Type: variant

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_deserialize.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_type_variant.h"

#include "icpp_hooks.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <utility>

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

CandidTypeVariant::CandidTypeVariant(const std::string label,
                                     const CandidType field)
    : CandidTypeBase() {

  m_label = label;
  initialize();
  append(label, field);
}

CandidTypeVariant::~CandidTypeVariant() {}

void CandidTypeVariant::initialize() {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVariant::set_datatype() {
  m_datatype_opcode = CandidOpcode().Variant;
  m_datatype_hex = OpcodeHex().Variant;
  m_datatype_textual = OpcodeTextual().Variant;
}

// Tuple notation with uint32_t field_id
void CandidTypeVariant::append(uint32_t field_id, CandidType field) {
  std::string s = "";
  _append(field_id, s, field);
}

// Variant notation with string name -> hash name into uint32_t field_id
void CandidTypeVariant::append(std::string field_name, CandidType field) {
  uint32_t field_id = idl_hash(field_name);
  _append(field_id, field_name, field);
}

// Tuple notation without field_id -> generate sequential uint32_t field_id
void CandidTypeVariant::append(CandidType field) {
  // The implicit id follows the previously appended field, not the largest id
  if (m_field_ptrs.size() == 0) {
    uint32_t field_id = 0;
    append(field_id, field);
  } else if (in_range<std::uint32_t>(m_last_appended_field_id + 1)) {
    uint32_t field_id = m_last_appended_field_id + 1;
    append(field_id, field);
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
                                CandidType field) {

  auto iter = std::find(begin(m_field_ids), end(m_field_ids), field_id);

  if (iter != end(m_field_ids) && field_name == m_label && !m_label_value_set) {
    // Erase the entry of the label, because it was not yet set by user
    auto i = std::distance(begin(m_field_ids), iter);
    m_field_ids.erase(m_field_ids.begin() + i);
    m_field_names.erase(m_field_names.begin() + i);
    m_field_datatypes.erase(m_field_datatypes.begin() + i);
    m_field_opcodes.erase(m_field_opcodes.begin() + i);
    m_field_content_datatypes.erase(m_field_content_datatypes.begin() + i);
    m_field_content_opcodes.erase(m_field_content_opcodes.begin() + i);
    m_field_ptrs.erase(m_field_ptrs.begin() + i);

    // Check again
    iter = std::find(begin(m_field_ids), end(m_field_ids), field_id);
  }

  if (iter != end(m_field_ids)) {
    auto i = std::distance(begin(m_field_ids), iter);
    std::string msg;
    msg.append("ERROR: variant already has a field with the same field_id.\n");
    msg.append("       field id (hash): " + std::to_string(field_id) + "\n");
    msg.append("       field name 1   : " + m_field_names[i] + "\n");
    msg.append("       field name 2   : " + field_name + "\n");
    ICPP_HOOKS::trap(msg);
  }
  // Add the field
  m_field_ids.push_back(field_id);
  m_field_names.push_back(field_name);
  m_last_appended_field_id = field_id;

  // During encoding, the datatype is always a negative opcode !
  // Store the negative opcode of the field's type
  int datatype =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, field);
  assert(datatype < 0);
  m_field_datatypes.push_back(datatype);
  m_field_opcodes.push_back(datatype);

  // Store the shared pointer to a CandidTypeRoot class
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

  if (field_name == m_label) {
    m_label_value_set = true;
  }

  sort_fields_by_id();

  encode_T();
  encode_M();
}

// (re-)build the type table encoding
void CandidTypeVariant::encode_T() {
  m_T.clear();

  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_uleb128(__uint128_t(m_field_ptrs.size()));
  for (size_t i = 0; i < m_field_ptrs.size(); ++i) {
    // id or hash of the variant field, append without packing into a fixed width
    m_T.append_uleb128(__uint128_t(m_field_ids[i]));

    // type table or the data type of the record field
    if (m_field_ptrs[i]) {
      VecBytes T = m_field_ptrs[i]->get_T();
      if (T.size() > 0) {
        // For <comptypes>, we use the index into the type table we defined above
        __uint128_t type_table_index = m_field_ptrs[i]->get_type_table_index();
        m_T.append_sleb128(__int128_t(type_table_index));
      } else {
        // For <primtypes>, use the Opcode, already stored
        VecBytes I = m_field_ptrs[i]->get_I();
        for (std::byte b : I.vec()) {
          m_T.append_byte(b);
        }
      }
    }
  }

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
  ;
}

// For variants, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVariant::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

// (re-)build the value encoding
void CandidTypeVariant::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(kv   : variant {<fieldtype>*}) = leb128(i) M(kv : <fieldtype>*[i])
  m_M.clear();

  for (size_t i = 0; i < m_field_names.size(); ++i) {
    if (m_field_names[i] == m_label) {
      // encode index of variant options - leb128(i)
      m_M.append_uleb128(__uint128_t(i));

      // encode the variant's value
      // The get_M method is in the CandidTypeBase class
      if (m_field_ptrs[i]) {
        VecBytes M = m_field_ptrs[i]->get_M();
        for (std::byte b : M.vec()) {
          m_M.append_byte(b);
        }
      }

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
