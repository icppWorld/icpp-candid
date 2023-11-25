// The class for the Candid Type: variant

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_deserialize.h"
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
  if (m_field_ptrs.size() == 0) {
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

  // Note: Unlike for a Record, the variant's fields are not sorted

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
        m_T.append_uleb128(type_table_index);
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

// Decode the type table, starting at & updating offset
bool CandidTypeVariant::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  std::string debug_hex_string;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVariant::decode_T");
    ICPP_HOOKS::debug_print("Start deserialization of a Variants' Type Table");
    debug_hex_string = B.as_hex_string();
  }
  m_field_ids.clear();
  m_field_names.clear();
  m_field_ptrs.clear();
  m_field_datatypes.clear();
  m_field_opcodes.clear();
  m_field_content_datatypes.clear();
  m_field_content_opcodes.clear();

  __uint128_t offset_start = offset;
  __uint128_t num_fields;
  __uint128_t numbytes;
  if (B.parse_uleb128(offset, num_fields, numbytes, parse_error)) {
    return true;
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    B.debug_print_as_hex_string(debug_hex_string, offset_start, offset);
    ICPP_HOOKS::debug_print("num_fields = " +
                            ICPP_HOOKS::to_string_128(num_fields));
  }

  for (size_t i = 0; i < num_fields; ++i) {
    // id or hash of the variant field
    offset_start = offset;
    __uint128_t field_id;
    __uint128_t numbytes;
    if (B.parse_uleb128(offset, field_id, numbytes, parse_error)) {
      return true;
    }
    m_field_ids.push_back(field_id);
    m_field_names.push_back("");
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      B.debug_print_as_hex_string(debug_hex_string, offset_start, offset);
      ICPP_HOOKS::debug_print("field " + std::to_string(i) + " - id (hash) = " +
                              ICPP_HOOKS::to_string_128(field_id));
    }

    // Get the datatype of the type table
    offset_start = offset;
    std::string parse_error;
    __int128_t datatype;
    numbytes = 0;
    if (B.parse_sleb128(offset, datatype, numbytes, parse_error)) {
      std::string to_be_parsed = "Type table: datatype";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    int field_datatype = int(datatype);
    // Cannot yet set opcode & content_, because we did not yet parse all the type tables
    int field_opcode = CANDID_UNDEF;
    m_field_datatypes.push_back(field_datatype);
    m_field_opcodes.push_back(CANDID_UNDEF);
    m_field_content_datatypes.push_back(CANDID_UNDEF);
    m_field_content_opcodes.push_back(CANDID_UNDEF);
    m_field_ptrs.push_back(nullptr);

    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      B.debug_print_as_hex_string(debug_hex_string, offset_start, offset);
      ICPP_HOOKS::debug_print(
          "field " + std::to_string(i) +
          " - datatype  = " + ICPP_HOOKS::to_string_128(datatype) + " (" +
          CandidOpcode().name_from_opcode(int(datatype)) + ")");
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
  }

  // Also set the _wire fields
  m_field_ids_wire = m_field_ids;
  m_field_ptrs_wire = m_field_ptrs;
  m_field_datatypes_wire = m_field_datatypes;
  m_field_opcodes_wire = m_field_opcodes;
  m_field_content_datatypes_wire = m_field_content_datatypes;
  m_field_content_opcodes_wire = m_field_content_opcodes;

  return false;
}

// During deserialization, after all type tables are read, this method is called
void CandidTypeVariant::finish_decode_T(CandidDeserialize &de) {
  for (size_t i = 0; i < m_field_ids.size(); ++i) {
    int field_datatype_wire = m_field_datatypes[i];
    int field_content_datatype_wire{CANDID_UNDEF};
    int field_content_opcode_wire{CANDID_UNDEF};

    int field_opcode_wire =
        de.get_opcode_from_datatype_on_wire(field_datatype_wire);
    if (field_datatype_wire >= 0) { // index into the type tables
      CandidTypeTable field_type_table_wire =
          de.get_typetables_wire()[field_datatype_wire];
      field_opcode_wire = field_type_table_wire.get_opcode();
      field_content_datatype_wire =
          field_type_table_wire.get_content_datatype();
      field_content_opcode_wire = field_type_table_wire.get_content_opcode();
    }

    // Create a dummy CandidType to use during decoding of the Variant's fields
    // Store a shared pointer to a CandidTypeRoot class
    std::shared_ptr<CandidTypeRoot> field_ptr_wire{nullptr};
    CandidOpcode candidOpcode;
    auto type_tables = de.get_typetables_wire();
    if (field_opcode_wire == candidOpcode.Vec) {
      CandidTypeTable *p_field_content_type_table =
          &type_tables[field_content_datatype_wire];
      field_ptr_wire = candidOpcode.candid_type_vec_from_opcode(
          field_content_opcode_wire, p_field_content_type_table);
      field_ptr_wire->set_content_datatype(field_content_datatype_wire);
    } else if (field_opcode_wire == candidOpcode.Opt) {
      CandidTypeTable *p_field_content_type_table =
          &type_tables[field_content_datatype_wire];
      field_ptr_wire = candidOpcode.candid_type_opt_from_opcode(
          field_content_opcode_wire, p_field_content_type_table);
      field_ptr_wire->set_content_datatype(field_content_datatype_wire);
    } else {
      field_ptr_wire = candidOpcode.candid_type_from_opcode(field_opcode_wire);
    }
    if (field_ptr_wire) field_ptr_wire->set_is_internal(true);

    m_field_opcodes[i] = field_opcode_wire;
    m_field_content_datatypes[i] = field_content_datatype_wire;
    m_field_content_opcodes[i] = field_content_opcode_wire;
    m_field_ptrs[i] = field_ptr_wire;

    m_field_opcodes_wire[i] = field_opcode_wire;
    m_field_content_datatypes_wire[i] = field_content_datatype_wire;
    m_field_content_opcodes_wire[i] = field_content_opcode_wire;
    m_field_ptrs_wire[i] = field_ptr_wire;
  }
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

// Decode the values, starting at & updating offset
bool CandidTypeVariant::decode_M(CandidDeserialize &de, VecBytes B,
                                 __uint128_t &B_offset,
                                 std::string &parse_error) {
  std::string debug_hex_string;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVariant::decode_M");
    debug_hex_string = B.as_hex_string();
  }

  if (m_field_ids.size() == 0) {
    ICPP_HOOKS::trap(
        "ERROR: m_field_ids has size 0 in CandidTypeRecord::decode_M");
  }

  // For a variant, there is only one field in the decode_M.
  // Notes:
  //   (-) The wire contains the index into the type-table on the wire, which is NOT
  //       necessarily the same as the expected type table. The type-table of
  //       fields on the wire can contain:
  //       (-) just the one of the stored variant itself (most likely) -> j=0
  //       (-) all the expected variant fields (only one is selected)
  //       (-) additional Opt fields (unlikely, but allowed)
  //
  //       So, we must do a check against the variant fields' label-hash.
  //
  //   (-) It is possible that decode_M is called for an additional Opt Variant.
  //       In that case, it is even more important to use the type table of the
  //       wire to decode
  //
  //   (-) Forward compatibility considerations also can result in very weird, but valid cases, like:
  //         '(opt (variant { Err = opt (variant { Err = "Error" : text}) }))'

  // Read index into the Variant's type-table on the wire - leb128(i)
  CandidOpcode candidOpcode;
  __uint128_t B_offset_start = B_offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t j128;
  if (B.parse_uleb128(B_offset, j128, numbytes, parse_error)) {
    std::string to_be_parsed = "index in variant options - leb128(i)";
    CandidAssert::trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                                        parse_error);
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    B.debug_print_as_hex_string(debug_hex_string, B_offset_start, B_offset);
    ICPP_HOOKS::debug_print("Field index into type table on wire = " +
                            ICPP_HOOKS::to_string_128(j128));
  }
  size_t j = static_cast<size_t>(j128);

  // match the variant's id (hash) of wire with passed in variant fields
  bool found_match{false};
  size_t i{0};
  for (i = 0; i < m_field_ptrs.size(); ++i) {
    if (m_field_ids_wire[j] == m_field_ids[i]) {
      found_match = true;
      m_label = m_field_names[i];
      // Fill the user's data placeholder, if a pointer was provided
      if (m_p_label) *m_p_label = m_label;
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
    for (size_t i = 0; i < m_field_ptrs.size(); ++i) {
      msg.append("       - " + std::to_string(m_field_ids[i]) + " (" +
                 m_field_names[i] + ")" + "\n");
    }
    ICPP_HOOKS::trap(msg);
  }

  // Check if all is OK and if there is anything to decode, with either expected or wire decoder
  std::shared_ptr<CandidTypeRoot> decoder = nullptr;
  std::string decoder_name;

  select_decoder_or_trap(de, i, j, decoder, decoder_name);

  if (!decoder) {
    if (decoder_name == "skip-expected-opt-not-found-on-wire") {
      // Expected Opt not found on the wire, but that is OK
    } else {
      ICPP_HOOKS::trap("ERROR: decoder for a variant field is a nullptr - " +
                       std::string(__func__));
    }
  } else {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg;
      msg.append("Start reading data by calling decode_M for (i,j) = ()" +
                 std::to_string(i) + "," + std::to_string(j) + ")");
      msg.append("\n- Using decode_M of " + decoder_name + " for Opcode " +
                 std::to_string(decoder->get_datatype_opcode()) + " (" +
                 decoder->get_datatype_textual() + ")");
      ICPP_HOOKS::debug_print(msg);
    }
    B_offset_start = B_offset;
    parse_error = "";
    if (decoder->decode_M(de, B, B_offset, parse_error)) {
      std::string to_be_parsed =
          "Values (decoding M) for argument at wire index " + std::to_string(j);
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
  }
  return false;
}

void CandidTypeVariant::select_decoder_or_trap(
    CandidDeserialize &de, size_t i, size_t j,
    std::shared_ptr<CandidTypeRoot> &decoder, std::string &decoder_name) {

  CandidOpcode candidOpcode;

  // The field on the wire (j)
  uint32_t field_id_wire = m_field_ids_wire[j];
  int field_datatype_wire = m_field_datatypes_wire[j];
  int field_opcode_wire = m_field_opcodes_wire[j];
  std::shared_ptr<CandidTypeRoot> field_ptr_wire = m_field_ptrs_wire[j];
  int field_content_opcode_wire{CANDID_UNDEF};
  std::string field_content_name_wire;
  if (field_opcode_wire == candidOpcode.Opt ||
      field_opcode_wire == candidOpcode.Vec) {
    field_content_opcode_wire = field_ptr_wire->get_content_opcode();
    field_content_name_wire =
        candidOpcode.name_from_opcode(field_content_opcode_wire);
  }

  // The expected field (i)
  uint32_t field_id_expected = m_field_ids[i];
  std::string field_name_expected = m_field_names[i];
  int field_datatype_expected = m_field_datatypes[i];
  int field_opcode_expected = m_field_opcodes[i];
  std::shared_ptr<CandidTypeRoot> field_ptr_expected = m_field_ptrs[i];
  int field_content_opcode_expected{CANDID_UNDEF};
  std::string field_content_name_expected;
  if (field_opcode_expected == candidOpcode.Opt ||
      field_opcode_expected == candidOpcode.Vec) {
    field_content_opcode_expected = field_ptr_expected->get_content_opcode();
    field_content_name_expected =
        candidOpcode.name_from_opcode(field_content_opcode_expected);
  }

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    std::string msg = "--";
    msg.append("\nexpected field has index " + std::to_string(i));
    msg.append("\n- Name (label) = " + field_name_expected);
    msg.append("\n- id   (hash)  = " + std::to_string(field_id_expected));
    msg.append("\n- Opcode       = " + std::to_string(field_opcode_expected) +
               " (" + candidOpcode.name_from_opcode(field_opcode_expected) +
               ")");
    if (field_opcode_expected == candidOpcode.Opt) {
      msg.append("\n- field_content_opcode_expected = " +
                 std::to_string(field_content_opcode_expected) + " (" +
                 field_content_name_expected + ")");
    }

    msg.append("\nfield on wire has index " + std::to_string(j));
    msg.append("\n- id   (hash)  = " + std::to_string(field_id_wire));
    msg.append("\n- datatype     = " + std::to_string(field_datatype_wire));
    msg.append("\n- Opcode       = " + std::to_string(field_opcode_wire) +
               " (" + candidOpcode.name_from_opcode(field_opcode_wire) + ")");
    if (field_datatype_wire >= 0) {
      msg.append("\n  (walked the type-tables to find that Opcode!)");
    }
    if (field_opcode_wire == candidOpcode.Opt) {
      msg.append("\n- opt_content_opcode_wire = " +
                 std::to_string(field_content_opcode_wire) + " (" +
                 field_content_name_wire + ")");
    }
    ICPP_HOOKS::debug_print(msg);
  }

  decoder = nullptr;
  bool field_error{false};
  if (field_id_wire == field_id_expected) {
    // field Ids (the hash) of expected & wire match
    // ==> Follow exact same logic as for args in candid_type_deserialize
    if (((field_opcode_wire != candidOpcode.Opt &&
          field_opcode_wire == field_opcode_expected) ||
         (field_opcode_wire == candidOpcode.Opt &&
          field_opcode_expected == candidOpcode.Opt &&
          field_content_opcode_wire == field_content_opcode_expected) ||
         (field_opcode_wire == candidOpcode.Vec &&
          field_opcode_expected == candidOpcode.Vec &&
          field_content_opcode_wire == field_content_opcode_expected))) {
      // Found an exact match => decode it with expected field and caller will forward wire index j
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is expected and found on wire.\n    We will decode M & store the data.");
      }
      decoder = field_ptr_expected;
      decoder_name = "expected";
      // Fill the field _wire info used by decode_M
      if ((field_opcode_expected == candidOpcode.Record) ||
          (field_opcode_expected == candidOpcode.Variant)) {
        // record { record } or record {variant}
        int field_datatype_wire = m_field_datatypes_wire[j];
        auto p_wire =
            de.get_typetables_wire()[field_datatype_wire].get_p_wire();
        decoder->set_fields_wire(p_wire);
      } else if (field_opcode_expected == candidOpcode.Opt &&
                 field_content_opcode_wire == candidOpcode.Vec) {
        // record { opt {vec} }
        CandidType c_decoder = decoder->toCandidType();
        CandidTypeOptVec *p_opt_vec = std::get_if<CandidTypeOptVec>(&c_decoder);
        if (p_opt_vec) {
          int field_content_datatype_wire = m_field_content_datatypes_wire[j];
          auto p_content_wire =
              de.get_typetables_wire()[field_content_datatype_wire]
                  .get_p_wire();
          if (p_content_wire) {
            if (p_content_wire->get_content_opcode() == candidOpcode.Record) {
              // OptVecRecord
              CandidType c_content_wire = p_content_wire->toCandidType();
              CandidTypeVecRecord *p_vec_record_wire =
                  std::get_if<CandidTypeVecRecord>(&c_content_wire);
              auto p_record_wire = p_vec_record_wire->get_pvs();

              CandidType c_content = p_opt_vec->get_pv()->toCandidType();
              CandidTypeVecRecord *p_vec_record =
                  std::get_if<CandidTypeVecRecord>(&c_content);
              p_vec_record->get_pvs()->set_fields_wire(p_record_wire);
            }
          } else {
            ICPP_HOOKS::trap(
                "ERROR: p_content_wire is a nullptr, likely a bug. for Variant field CandidTypeOptVecRecord - " +
                std::string(__func__));
          }
        }
      } else if (field_opcode_expected == candidOpcode.Opt &&
                 field_content_opcode_wire == candidOpcode.Record) {
        int field_content_datatype_wire = m_field_content_datatypes_wire[j];
        auto p_content_wire =
            de.get_typetables_wire()[field_content_datatype_wire].get_p_wire();
        CandidType c_content_wire = p_content_wire->toCandidType();
        CandidTypeRecord *p_content_record =
            std::get_if<CandidTypeRecord>(&c_content_wire);

        CandidType c_decoder = decoder->toCandidType();
        CandidTypeOptRecord *p_opt_record =
            std::get_if<CandidTypeOptRecord>(&c_decoder);
        if (p_opt_record && p_content_record) {
          p_opt_record->get_pv()->set_fields_wire(p_content_wire);
        } else {
          ICPP_HOOKS::trap(
              "ERROR: Unexpected type-table for Variant field CandidTypeOptRecord - " +
              std::string(__func__));
        }
      } else if (field_opcode_expected == candidOpcode.Opt &&
                 field_content_opcode_wire == candidOpcode.Variant) {
        int field_content_datatype_wire = m_field_content_datatypes_wire[j];
        auto p_content_wire =
            de.get_typetables_wire()[field_content_datatype_wire].get_p_wire();
        CandidType c_content_wire = p_content_wire->toCandidType();
        CandidTypeVariant *p_content_variant =
            std::get_if<CandidTypeVariant>(&c_content_wire);

        CandidType c_decoder = decoder->toCandidType();
        CandidTypeOptVariant *p_opt_variant =
            std::get_if<CandidTypeOptVariant>(&c_decoder);
        if (p_opt_variant && p_content_variant) {
          p_opt_variant->get_pv()->set_fields_wire(p_content_wire);
        } else {
          ICPP_HOOKS::trap(
              "ERROR: Unexpected type-table for Variant field CandidTypeOptVariant - " +
              std::string(__func__));
        }
      } else if ((field_content_opcode_wire == candidOpcode.Record) ||
                 (field_content_opcode_wire == candidOpcode.Variant)) {
        ICPP_HOOKS::trap(
            "ERROR: We do NOT yet handle this field type: \n " +
            std::to_string(field_opcode_wire) + " (" +
            candidOpcode.name_from_opcode(field_opcode_wire) +
            std::to_string(field_content_opcode_wire) + " (" +
            candidOpcode.name_from_opcode(field_content_opcode_wire) + " - " +
            std::string(__func__));
      }
    } else if (field_opcode_wire == candidOpcode.Null &&
               field_opcode_expected == candidOpcode.Opt) {
      // Special case, Expected opt found as '(null)'
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an expected Opt and found as 'null' on wire.\n    There is nothing to decode.");
      }
      decoder = nullptr; // Nothing to decode
      decoder_name = "expected-opt-as-null";
    } else if (field_opcode_wire == candidOpcode.Opt &&
               field_opcode_expected == candidOpcode.Opt &&
               field_content_opcode_wire == candidOpcode.Null) {
      // Special case, Expected opt found as '(opt (null : null))'
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an expected Opt and found as '(opt (null : null))' on wire.\n    We will decode M & discard the data.");
      }
      decoder = build_decoder_wire_for_additional_opt_field(de, j);
      decoder_name = "expected-opt-as-opt-null";
    } else if (field_opcode_wire == candidOpcode.Opt) {
      // Additional Opt on wire must be decoded and discarded
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an unexpected additional Opt found on wire.\n    We will decode M & discard the data.");
      }
      decoder = build_decoder_wire_for_additional_opt_field(de, j);
      decoder_name = "read-and-discard-additional-wire-opt";
    } else if (field_opcode_expected == candidOpcode.Opt) {
      // Expected Opt not found on wire. Skip it and continue
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an expected Opt that we cannot find on the wire.\n    We will skip the expected arg.");
      }
      decoder = nullptr;
      decoder_name = "skip-expected-opt-not-found-on-wire";
    } else {
      // Wrong type on wire
      // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
      field_error = true;
    }
  } else {
    // field Ids (the hash) of expected & wire do not match
    // -> Only allowed if one or both are Opt fields

    // Is the field_id on the wire one of the next expected fields?
    // - yes: only allowed if expected is an Opt, which has been omitted
    // - no : only allowed if wire is an addional Opt, which must be read & discarded

    bool found{false};
    for (auto k = i + 1; k < m_field_ids.size(); ++k) {
      if (field_id_wire == m_field_ids[k]) {
        found = true;
        break;
      }
    }
    if (found && field_opcode_expected == candidOpcode.Opt) {
      // Expected Opt not found on wire. Skip it and continue
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an expected Opt that we cannot find on the wire.\n    We will skip the expected arg.");
      }
      decoder = nullptr;
      decoder_name = "skip-expected-opt-not-found-on-wire";
    } else if (!found && field_opcode_wire == candidOpcode.Opt) {
      // Additional Opt on wire must be decoded and discarded
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "--> This field is an unexpected additional Opt found on wire.\n    We will decode M & discard the data.");
      }
      decoder = build_decoder_wire_for_additional_opt_field(de, j);
      decoder_name = "read-and-discard-additional-wire-opt";
    } else {
      // Wrong type on wire
      // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
      field_error = true;
    }
  }

  if (field_error) {
    std::string msg;
    msg.append("ERROR: Wrong Variant field on wire:\n");
    msg.append("\nexpected field has index " + std::to_string(i));
    msg.append("\n- Opcode   = " + std::to_string(field_opcode_expected) +
               " (" + candidOpcode.name_from_opcode(field_opcode_expected) +
               ")");
    if (field_opcode_expected == candidOpcode.Opt) {
      msg.append("\n- field_content_opcode_expected = " +
                 std::to_string(field_content_opcode_expected) + " (" +
                 field_content_name_expected + ")");
    }

    msg.append("\nfield on wire has index " + std::to_string(j));
    msg.append("\n- datatype = " + std::to_string(field_datatype_wire));
    msg.append("\n- Opcode   = " + std::to_string(field_opcode_wire) + " (" +
               candidOpcode.name_from_opcode(field_opcode_wire) + ")");
    if (field_datatype_wire >= 0) {
      msg.append("\n  (walked the type-tables to find that Opcode!)");
    }
    if (field_opcode_wire == candidOpcode.Opt) {
      msg.append("\n- opt_content_opcode_wire = " +
                 std::to_string(field_content_opcode_wire) + " (" +
                 field_content_name_wire + ")");
    }
    ICPP_HOOKS::trap(msg);
  }
}

std::shared_ptr<CandidTypeRoot>
CandidTypeVariant::build_decoder_wire_for_additional_opt_field(
    CandidDeserialize &de, int j) {
  // Build the decoder for an additional opt field on the wire
  CandidOpcode candidOpcode;
  int field_datatype_wire = m_field_datatypes_wire[j];
  int field_opcode_wire = m_field_opcodes_wire[j];
  int field_content_datatype_wire = m_field_content_datatypes_wire[j];
  int field_content_opcode_wire = m_field_content_opcodes_wire[j];

  if (field_opcode_wire != candidOpcode.Opt) {
    ICPP_HOOKS::trap(
        "ERROR: this method should only be called for an additional Opt field on the wire.");
  }

  std::shared_ptr<CandidTypeRoot> p_wire{nullptr};
  if (field_datatype_wire >= 0) {
    // For types with a type-table, like Record & Variant,
    // the decoder was created during deserialization of the type table section
    p_wire = de.get_typetables_wire()[field_datatype_wire].get_p_wire();
  } else if (field_content_opcode_wire == CANDID_UNDEF) {
    ICPP_HOOKS::trap(
        "ERROR: cannot build the decoder_wire because field_content_opcode_wire is not set.");
  } else {
    CandidTypeTable *p_field_content_type_table = nullptr;
    if (field_datatype_wire >= 0) {
      p_field_content_type_table =
          &de.get_typetables_wire()[field_datatype_wire];
    }
    p_wire = candidOpcode.candid_type_opt_from_opcode(
        field_content_opcode_wire, p_field_content_type_table);
    p_wire->set_content_datatype(field_content_opcode_wire);
    p_wire->set_is_internal(true);
  }

  return p_wire;
}

void CandidTypeVariant::set_fields_wire(
    std::shared_ptr<CandidTypeRoot> p_from_wire) {
  // Move the type table data of the Variant that we found on the wire
  // into vectors with the corresponding Variant we are expecting
  // We do all the checking on correctness in decode_M
  m_field_ids_wire.clear();
  m_field_datatypes_wire.clear();
  m_field_opcodes_wire.clear();
  m_field_content_datatypes_wire.clear();
  m_field_content_opcodes_wire.clear();
  m_field_ptrs_wire.clear();

  // For additional opts, also set m_field_ids, so decoding checks pass
  bool add_opt{false};
  if (m_field_ids.size() == 0) {
    add_opt = true;
    m_field_ids.clear();
    m_field_names.clear();
    m_field_datatypes.clear();
    m_field_opcodes.clear();
    m_field_content_datatypes.clear();
    m_field_content_opcodes.clear();
    m_field_ptrs.clear();
  }

  for (size_t i = 0; i < p_from_wire->get_field_ids().size(); ++i) {
    m_field_ids_wire.push_back(p_from_wire->get_field_ids()[i]);
    m_field_datatypes_wire.push_back(p_from_wire->get_field_datatypes()[i]);
    m_field_opcodes_wire.push_back(p_from_wire->get_field_opcodes()[i]);
    m_field_content_datatypes_wire.push_back(
        p_from_wire->get_field_content_datatypes()[i]);
    m_field_content_opcodes_wire.push_back(
        p_from_wire->get_field_content_opcodes()[i]);
    m_field_ptrs_wire.push_back(p_from_wire->get_field_ptrs()[i]);

    if (add_opt) {
      m_field_ids.push_back(p_from_wire->get_field_ids()[i]);
      m_field_names.push_back("");
      m_field_datatypes.push_back(p_from_wire->get_field_datatypes()[i]);
      m_field_opcodes.push_back(p_from_wire->get_field_opcodes()[i]);
      m_field_content_datatypes.push_back(
          p_from_wire->get_field_content_datatypes()[i]);
      m_field_content_opcodes.push_back(
          p_from_wire->get_field_content_opcodes()[i]);
      m_field_ptrs.push_back(p_from_wire->get_field_ptrs()[i]);
    }
  }
}
