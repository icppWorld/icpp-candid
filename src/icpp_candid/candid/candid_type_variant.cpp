// The class for the Candid Type: variant

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_type_variant.h"

#include "pro.h"

#include "icpp_hooks.h"

#include <algorithm>
#include <cassert>
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
  if (m_fields_ptrs.size() == 0) {
    uint32_t field_id = 0;
    append(field_id, field);
  }

  if (std::in_range<std::uint32_t>(m_field_ids.back() + 1)) {
    uint32_t field_id = m_field_ids.back() + 1;
    append(field_id, field);
  } else {
    std::string msg;
    msg.append("ERROR: field_id outside range of uint32_t\n");
    msg.append("       id of previous field in variant: " +
               std::to_string(m_field_ids.back()) + "\n");
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
    m_fields_ptrs.erase(m_fields_ptrs.begin() + i);

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
  int datatype =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, field);
  m_field_datatypes.push_back(datatype);
  // Store the shared pointer to a CandidTypeBase class
  m_fields_ptrs.push_back(std::visit(
      [](auto &&arg) -> std::shared_ptr<CandidTypeRoot> {
        return std::make_shared<std::decay_t<decltype(arg)>>(arg);
      },
      field));

  if (field_name == m_label) {
    m_label_value_set = true;
  }

  encode_T();
  encode_M();
}

// (re-)build the type table encoding
void CandidTypeVariant::encode_T() {
  m_T.clear();

  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_uleb128(__uint128_t(m_fields_ptrs.size()));
  for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
    // id or hash of the variant field, append without packing into a fixed width
    m_T.append_uleb128(__uint128_t(m_field_ids[i]));

    // type table or the data type of the record field
    if (m_fields_ptrs[i]) {
      VecBytes T = m_fields_ptrs[i]->get_T();
      if (T.size() > 0) {
        // For <comptypes>, we use the index into the type table we defined above
        __uint128_t type_table_index = m_fields_ptrs[i]->get_type_table_index();
        m_T.append_uleb128(type_table_index);
      } else {
        // For <primtypes>, use the Opcode, already stored
        VecBytes I = m_fields_ptrs[i]->get_I();
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

// Decode the type table, starting at & updating offset
bool CandidTypeVariant::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVariant::decode_T");
    ICPP_HOOKS::debug_print("Start deserialization of a Variants' Type Table");
    ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
  }
  m_field_ids.clear();
  m_field_names.clear();
  m_fields_ptrs.clear();
  m_field_datatypes.clear();

  __uint128_t num_fields;
  __uint128_t numbytes;
  if (B.parse_uleb128(offset, num_fields, numbytes, parse_error)) {
    return true;
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("num_fields = " +
                            ICPP_HOOKS::to_string_128(num_fields));
    ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
  }

  for (size_t i = 0; i < num_fields; ++i) {
    // id or hash of the variant field
    __uint128_t field_id;
    __uint128_t numbytes;
    if (B.parse_uleb128(offset, field_id, numbytes, parse_error)) {
      return true;
    }
    m_field_ids.push_back(field_id);
    m_field_names.push_back("");
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print("field " + std::to_string(i) + " - id (hash) = " +
                              ICPP_HOOKS::to_string_128(field_id));
      ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
    }

    // Get the datatype of the type table
    __uint128_t offset_start = offset;
    std::string parse_error;
    __int128_t datatype;
    numbytes = 0;
    if (B.parse_sleb128(offset, datatype, numbytes, parse_error)) {
      std::string to_be_parsed = "Type table: datatype";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_field_datatypes.push_back(int(datatype));
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "field " + std::to_string(i) +
          " - datatype  = " + ICPP_HOOKS::to_string_128(datatype) + " (" +
          CandidOpcode().name_from_opcode(int(datatype)) + ")");
      ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
    }

    // We only use this decode_T for a dummy CandidType during type table deserialization
    // No need to store a reference to a CandidType, it will never be used.
    m_fields_ptrs.push_back(nullptr);
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
  }
  return false;
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
      if (m_fields_ptrs[i]) {
        VecBytes M = m_fields_ptrs[i]->get_M();
        for (std::byte b : M.vec()) {
          m_M.append_byte(b);
        }
      }

      break;
    }
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVariant::decode_M(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  // get index of variant options - leb128(i)
  // Note: It is allowed that Candid contains ONLY the variant label-hash/type/value itself.
  //       In that case, the index will always be 0
  //       We have to check on the variant label-hash, not on the full variant index.
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t j;
  if (B.parse_uleb128(offset, j, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of vec- leb128(N)";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  // match the variant's id (hash) of wire with passed in variant fields
  bool found_match{false};
  for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
    if (m_field_ids_wire[j] == m_field_ids[i]) {
      found_match = true;
      m_label = m_field_names[i];
      m_variant_index = i;
      m_variant_index_wire = j;
      break;
    }
  }
  if (!found_match) {
    std::string msg;
    msg.append(
        "ERROR: The id (hash) of the Variant's value on the wire does not match any of the expected Variant ids.\n");
    msg.append("       The id (hash) on wire: " +
               std::to_string(m_field_ids_wire[j]) + "\n");
    msg.append("       Expecting one of the following ids:\n");
    for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
      msg.append("       - " + std::to_string(m_field_ids[i]) + " (" +
                 m_field_names[i] + ")" + "\n");
    }
    ICPP_HOOKS::trap(msg);
  }
  // verify that the types match
  int datatype = m_field_datatypes[m_variant_index];
  int datatype_wire = m_field_datatypes_wire[m_variant_index_wire];
  if (datatype != datatype_wire) {
    std::string msg;
    msg.append(
        "ERROR: the datatype of the Variant's value on the wire does not match the expected Variant datatype.\n");
    msg.append("       The id (hash) on wire: " +
               std::to_string(m_field_ids_wire[m_variant_index_wire]) + " (" +
               m_label + ")" + "\n");
    msg.append("       expected datatype for this id (hash): " +
               std::to_string(datatype) + " (" +
               CandidOpcode().name_from_opcode(datatype) + ")" + "\n");
    msg.append("       datatype on wire                    : " +
               std::to_string(datatype_wire) + " (" +
               CandidOpcode().name_from_opcode(datatype_wire) + ")" + "\n");
    ICPP_HOOKS::trap(msg);
  }

  // decode the value
  // TODO: what was this for?
  // if (datatype_wire == CandidOpcode().Null) {
  //   // There is no value to decode
  // } else {
  // We can just call the decode_M method on the base class, use runtime polymorphism
  // (-) The method decode_M is declared as a virtual function the base class
  // (-) It is implemented for ALL derived classes
  parse_error = "";
  offset_start = offset;

  if (m_fields_ptrs[m_variant_index]) {
    if (m_fields_ptrs[m_variant_index]->decode_M(B, offset, parse_error)) {
      std::string to_be_parsed = "Value for a Variant";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
  }
  // }
  // else {
  //   ICPP_HOOKS::trap(
  //       "TODO: Implement decode for non primitive type as a variant field, using recursion " +
  //       std::to_string(datatype) + std::string(__func__));
  // }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_p_label) *m_p_label = m_label;

  return false;
}

void CandidTypeVariant::check_type_table(const CandidTypeVariant *p_from_wire) {
  m_field_ids_wire.clear();
  m_field_datatypes_wire.clear();
  for (size_t i = 0; i < p_from_wire->m_fields_ptrs.size(); ++i) {
    m_field_ids_wire.push_back(p_from_wire->m_field_ids[i]);
    m_field_datatypes_wire.push_back(p_from_wire->m_field_datatypes[i]);
  }
}
