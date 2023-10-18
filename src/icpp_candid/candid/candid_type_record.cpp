// The class for the Candid Type: Record

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_type_record.h"

#include "pro.h"

#include "icpp_hooks.h"

#include <algorithm>
#include <cassert>
#include <utility>

CandidTypeRecord::CandidTypeRecord() : CandidTypeBase() {

  set_datatype();
  encode_T();
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

// Tuple notation without field_id -> generate sequential uint32_t field_id
void CandidTypeRecord::append(CandidType field) {
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
  int datatype =
      std::visit([](auto &&c) { return c.get_datatype_opcode(); }, field);
  m_field_datatypes.push_back(datatype);
  // Store a shared pointer to the CandidTypeRoot class
  m_fields_ptrs.push_back(std::visit(
      [](auto &&arg) -> std::shared_ptr<CandidTypeRoot> {
        return std::make_shared<std::decay_t<decltype(arg)>>(arg);
      },
      field));

  // Sort by field_id (hash)
  for (std::size_t i = 0; i < m_field_ids.size(); ++i) {
    for (std::size_t j = i + 1; j < m_field_ids.size(); ++j) {
      if (m_field_ids[i] > m_field_ids[j]) {
        auto temp_field_id = std::move(m_field_ids[i]);
        m_field_ids[i] = std::move(m_field_ids[j]);
        m_field_ids[j] = std::move(temp_field_id);

        auto temp_field_name = std::move(m_field_names[i]);
        m_field_names[i] = std::move(m_field_names[j]);
        m_field_names[j] = std::move(temp_field_name);

        auto temp_field = std::move(m_fields_ptrs[i]);
        m_fields_ptrs[i] = std::move(m_fields_ptrs[j]);
        m_fields_ptrs[j] = std::move(temp_field);

        auto temp_field_datatype = std::move(m_field_datatypes[i]);
        m_field_datatypes[i] = std::move(m_field_datatypes[j]);
        m_field_datatypes[j] = std::move(temp_field_datatype);
      }
    }
  }

  encode_T();
  encode_M();
}

// (re-)build the type table encoding
void CandidTypeRecord::encode_T() {
  m_T.clear();

  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_uleb128(__uint128_t(m_fields_ptrs.size()));
  for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
    // id or hash of the record field, append without packing into a fixed width
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
}

// Decode the type table, starting at & updating offset
bool CandidTypeRecord::decode_T(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeRecord::decode_T");
    ICPP_HOOKS::debug_print("Start deserialization of a Records' Type Table");
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
    // id or hash of the record field
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

  for (auto p_field : m_fields_ptrs) {
    // The get_M method is in the CandidTypeBase class
    if (p_field) {
      VecBytes M = p_field->get_M();
      for (std::byte b : M.vec()) {
        m_M.append_byte(b);
      }
    }
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeRecord::decode_M(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
    // TODO: what was this for?
    // if (m_field_datatypes_wire[i] == CandidOpcode().Null) {
    //   // There is no value to decode
    //   continue;
    // }

    parse_error = "";
    __uint128_t offset_start = offset;
    if (m_fields_ptrs[i]) {
      if (m_fields_ptrs[i]->decode_M(B, offset, parse_error)) {
        std::string to_be_parsed =
            "Value for a Record field at index " + std::to_string(i);
        CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                            parse_error);
      }
    } else {
      ICPP_HOOKS::trap("ERROR: A records' m_fields_ptrs[" + std::to_string(i) +
                       "] is a nullptr. The datatype on the wire is " +
                       std::to_string(m_field_datatypes_wire[i]));
    }
  }

  return false;
}

// Traps if the type table does not match the type table on the wire
void CandidTypeRecord::check_type_table(const CandidTypeRecord *p_from_wire) {
  m_field_datatypes_wire.clear();
  for (size_t i = 0; i < m_fields_ptrs.size(); ++i) {
    // id or hash of the record field
    uint32_t id = m_field_ids[i];
    uint32_t id_wire = p_from_wire->m_field_ids[i];
    if (id != id_wire) {
      std::string msg;
      msg.append("ERROR: the hashed id for the Record field at index " +
                 std::to_string(i) + " is wrong on the wire.\n");
      msg.append("       expected value of the hashed id: " +
                 std::to_string(id) + " (" + m_field_names[i] + ")" + "\n");
      msg.append("       found on wire  : " + std::to_string(id_wire) + "\n");
      ICPP_HOOKS::trap(msg);
    }

    int datatype = m_field_datatypes[i];
    int datatype_wire = p_from_wire->m_field_datatypes[i];
    m_field_datatypes_wire.push_back(
        datatype_wire); // save it for use in decode_M

    if (datatype != datatype_wire) {
      if (CandidOpcode().is_constype(datatype) && datatype_wire > 0) {
        // This is ok. The wire contains a type table, which is correct
        // TODO: we should walk the type table to check it is all consistent
      } else if (datatype_wire == CandidOpcode().Null &&
                 datatype == CandidOpcode().Opt) {
        // This is ok. A null is passed for an Opt
      } else {
        std::string msg;
        msg.append("ERROR: the datatype for the Record field at index " +
                   std::to_string(i) + " is wrong on the wire.\n");
        msg.append("       expected datatype: " + std::to_string(datatype) +
                   " (" + CandidOpcode().name_from_opcode(datatype) + ")" +
                   "\n");
        msg.append("       type on wire : " + std::to_string(datatype_wire) +
                   " (" + CandidOpcode().name_from_opcode(datatype_wire) + ")" +
                   "\n");
        ICPP_HOOKS::trap(msg);
      }
    }
  }
}
