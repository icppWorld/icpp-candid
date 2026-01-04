// The class for the Candid Type: vec variant (unit variants only)
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"
#include "candid_deserialize.h"

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_variant.h"
#include "candid_type_vec_variant.h"

#include "icpp_hooks.h"

#include <cassert>

CandidTypeVecVariant::CandidTypeVecVariant() : CandidTypeBase() {
  CandidTypeVariant v;
  set_v_template(v);
  initialize();
}

// Constructor for encoding: takes a template variant and vector of labels
CandidTypeVecVariant::CandidTypeVecVariant(
    const CandidTypeVariant &v_template, const std::vector<std::string> &labels)
    : CandidTypeBase() {
  set_v_template(v_template);
  m_labels = labels;
  m_v_size = labels.size();
  initialize();
}

// Constructor for decoding: pointer to receive labels
CandidTypeVecVariant::CandidTypeVecVariant(CandidTypeVariant *p_v_template,
                                           std::vector<std::string> *p_labels)
    : CandidTypeBase() {
  set_pv(static_cast<CandidTypeRoot *>(p_v_template));
  m_p_labels = p_labels;

  const CandidTypeVariant v = const_cast<CandidTypeVariant &>(*p_v_template);
  set_v_template(v);
  initialize();
}

// Internal: During deserialization for an expected argument
CandidTypeVecVariant::CandidTypeVecVariant(CandidTypeRoot *p_v_root)
    : CandidTypeBase() {
  set_pv(p_v_root);

  CandidTypeVariant *p_v_variant = dynamic_cast<CandidTypeVariant *>(p_v_root);
  if (p_v_variant) {
    const CandidTypeVariant v = const_cast<CandidTypeVariant &>(*p_v_variant);
    set_v_template(v);
    initialize();
  } else {
    ICPP_HOOKS::trap(
        "ERROR: p_v_root is not pointing to a CandidTypeVariant object - " +
        std::string(__func__));
  }
}

// Internal: During deserialization for an additional wire argument
CandidTypeVecVariant::CandidTypeVecVariant(
    std::shared_ptr<CandidTypeRoot> p_wire_variant)
    : CandidTypeBase() {
  m_pvs = p_wire_variant;
  set_datatype();
  set_content_type();
}

CandidTypeVecVariant::~CandidTypeVecVariant() {}

void CandidTypeVecVariant::push_back_value(CandidTypeRoot &value) {
  auto &derived_value = static_cast<CandidTypeVariant &>(value);
  std::string label = derived_value.get_label();

  // If label is empty, the variant was decoded internally without named fields.
  // Use the selected_field_id to look up the field name in our template.
  if (label.empty()) {
    uint32_t selected_id = derived_value.get_selected_field_id();
    if (selected_id != 0) {
      // Look up in our template variant
      auto template_field_ids = m_v_template.get_field_ids();
      auto template_field_names = m_v_template.get_v();
      for (size_t i = 0; i < template_field_ids.size(); ++i) {
        if (template_field_ids[i] == selected_id) {
          label = template_field_names[i];
          break;
        }
      }
    }
  }

  if (m_p_labels) {
    // We're deserializing with a pointer to user's vector
    m_p_labels->push_back(label);
  } else {
    // We're serializing or deserializing without pointer
    m_labels.push_back(label);
  }
}

void CandidTypeVecVariant::set_content_type() {
  m_content_opcode = CandidOpcode().Variant;
  m_content_hex = OpcodeHex().Variant;
  m_content_textual = OpcodeTextual().Variant;
}

int CandidTypeVecVariant::find_label_index(const std::string &label) {
  auto field_names = m_v_template.get_v();
  for (size_t i = 0; i < field_names.size(); ++i) {
    if (field_names[i] == label) {
      return static_cast<int>(i);
    }
  }
  return -1; // Not found
}

std::string CandidTypeVecVariant::find_label_name(size_t index) {
  auto field_names = m_v_template.get_v();
  if (index < field_names.size()) {
    return field_names[index];
  }
  return ""; // Not found
}

void CandidTypeVecVariant::create_dummy_variant_encode() {
  if (m_pv_dummy) {
    ICPP_HOOKS::trap("ERROR: This method should be called only once - " +
                     std::string(__func__));
  }
  // Create a fresh variant with the same fields, which will register it
  m_pv_dummy = std::make_shared<CandidTypeVariant>();

  // Copy the field structure from the template
  auto field_names = m_v_template.get_v();
  for (const auto &name : field_names) {
    // For unit variants, append with null type
    m_pv_dummy->append(name, CandidTypeNull{});
  }

  // Store as m_pvs for type table access
  m_pvs = m_pv_dummy;
}

// build the type table encoding
void CandidTypeVecVariant::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // Build a dummy Variant which will also create a type-table entry in the registry
  create_dummy_variant_encode();

  // Ok, so now we have a type-table in the registry for the Variant
  //     that we can reference from our Vec-of-Variants type table
  int variant_type_table_index = m_pvs->get_type_table_index();
  m_T.append_sleb128(__int128_t(variant_type_table_index));

  // Update the type table registry
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

void CandidTypeVecVariant::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N
  // For unit variants: M(kv : variant {<fieldtype>*}) = leb128(i)
  // (no additional data since all fields are null/unit types)

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_labels.size()));

  // encoded vec values - for each label, encode its index
  for (const auto &label : m_labels) {
    int index = find_label_index(label);
    if (index < 0) {
      std::string msg;
      msg.append("ERROR: Label '" + label +
                 "' not found in variant template options.");
      msg.append("\nValid options are:");
      for (const auto &name : m_v_template.get_v()) {
        msg.append("\n  - " + name);
      }
      ICPP_HOOKS::trap(msg);
    }
    // Encode variant index
    m_M.append_uleb128(__uint128_t(index));
    // No additional data for unit variants (null type)
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecVariant::decode_M(CandidDeserialize &de, VecBytes B,
                                    __uint128_t &offset,
                                    std::string &parse_error) {
  std::string debug_hex_string;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVecVariant::decode_M");
    debug_hex_string = B.as_hex_string();
  }

  // get size of vec - leb128(N)
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t size_vec_wire;
  if (B.parse_uleb128(offset, size_vec_wire, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of vec- leb128(N)";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    std::string msg;
    msg.append("Size of CandidTypeVecVariant on wire = " +
               ICPP_HOOKS::to_string_128(size_vec_wire));
    ICPP_HOOKS::debug_print(msg);
  }

  m_labels.clear();

  // Get field names from the wire variant if available, otherwise from template
  std::vector<std::string> field_names;
  if (m_pvs) {
    field_names = m_pvs->get_field_names();
  } else if (m_pv) {
    CandidTypeVariant *p_v_variant = dynamic_cast<CandidTypeVariant *>(m_pv);
    if (p_v_variant) {
      field_names = p_v_variant->get_v();
    }
  }
  if (field_names.empty()) {
    field_names = m_v_template.get_v();
  }

  // Loop over the vector size just read from the wire
  for (size_t i = 0; i < size_vec_wire; ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg;
      msg.append("Reading variant index for vec element " + std::to_string(i));
      ICPP_HOOKS::debug_print(msg);
    }

    // Read variant index
    offset_start = offset;
    __uint128_t variant_index;
    if (B.parse_uleb128(offset, variant_index, numbytes, parse_error)) {
      std::string to_be_parsed = "Variant index - leb128(i)";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }

    // Map index to label name
    size_t idx = static_cast<size_t>(variant_index);
    if (idx >= field_names.size()) {
      std::string msg;
      msg.append("ERROR: Variant index " + std::to_string(idx) +
                 " out of range. Max index is " +
                 std::to_string(field_names.size() - 1));
      ICPP_HOOKS::trap(msg);
    }

    std::string label = field_names[idx];
    m_labels.push_back(label);

    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg;
      msg.append("Decoded variant index " + std::to_string(idx) +
                 " as label '" + label + "'");
      ICPP_HOOKS::debug_print(msg);
    }

    // For unit variants, there's no additional data to decode
    // (the variant value is null/unit type)
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_p_labels) {
    *m_p_labels = m_labels;
  }

  return false;
}

// Initialize things
void CandidTypeVecVariant::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecVariant::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecVariant::decode_T(VecBytes B, __uint128_t &offset,
                                    std::string &parse_error) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVecVariant::decode_T");
    ICPP_HOOKS::debug_print(
        "Start deserialization of a VecVariant's Type Table");
    ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
  }

  __uint128_t len = B.size() - offset;

  // The opcode for content type, which is a type table index for a Variant
  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type;
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: a Vec's content type";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  m_content_opcode = int(content_type);
  return false;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecVariant::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
