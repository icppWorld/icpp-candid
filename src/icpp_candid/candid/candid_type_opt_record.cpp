// The class for the Candid Type: opt
#include "candid_debug_config.h"

#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_base.h"
#include "candid_deserialize.h"
#include "candid_type_opt_record.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptRecord::CandidTypeOptRecord() : CandidTypeBase() {
  CandidTypeRecord v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptRecord::CandidTypeOptRecord(CandidTypeRecord *p_v, bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire

  set_pv(static_cast<CandidTypeRoot *>(p_v));

  const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v);
  set_v(v);
  initialize();
}

// For internal use only, to create dummy instances for decoding during deserialization,
CandidTypeOptRecord::CandidTypeOptRecord(CandidTypeRoot *p_v_root,
                                         bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire
  set_pv(p_v_root);

  CandidTypeRecord *p_v_record = dynamic_cast<CandidTypeRecord *>(p_v_root);
  if (p_v_record) {
    // The cast was successful, it is indeed a Record
    const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v_record);
    set_v(v);
    initialize();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_root is not pointing to a CandidTypeRecord object - " +
        std::string(__func__));
  }
}

// These constructors are only for encoding
CandidTypeOptRecord::CandidTypeOptRecord(const CandidTypeRecord v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

// Internal: During deserialization for an additional wire argument (dummy !)
CandidTypeOptRecord::CandidTypeOptRecord(
    std::shared_ptr<CandidTypeRoot> p_v_root)
    : CandidTypeBase() {
  m_pvs = p_v_root;
  set_pv(p_v_root.get());
  CandidTypeRecord *p_v_record =
      dynamic_cast<CandidTypeRecord *>(p_v_root.get());
  if (p_v_record) {
    // The cast was successful, it is indeed a Record
    const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v_record);
    set_v(v);
    initialize();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_root is not pointing to a CandidTypeRecord object - " +
        std::string(__func__));
  }
}

CandidTypeOptRecord::~CandidTypeOptRecord() {}

void CandidTypeOptRecord::set_content_type() {
  m_content_opcode = CandidOpcode().Record;
  m_content_hex = OpcodeHex().Record;
  m_content_textual = OpcodeTextual().Record;
}

void CandidTypeOptRecord::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  m_M.append_int_fixed_width(uint8_t{1});
  m_M.append(m_v.get_M());
}

// Decode the values, starting at & updating offset
bool CandidTypeOptRecord::decode_M(CandidDeserialize &de, VecBytes B,
                                   __uint128_t &offset,
                                   std::string &parse_error) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  std::string debug_hex_string;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeOptRecord::decode_M");
    debug_hex_string = B.as_hex_string();
  }

  // get tag of opt - i8(0)
  // this will also work if there is a null on the wire.
  __uint128_t offset_start = offset;
  parse_error = "";
  uint8_t tag;
  if (B.parse_int_fixed_width(offset, tag, parse_error)) {
    std::string to_be_parsed = "Opt tag.";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    B.debug_print_as_hex_string(debug_hex_string, offset_start, offset);
    ICPP_HOOKS::debug_print("tag = " + std::to_string(tag));
  }
  if (tag == 1) {
    // Found it on the wire.
    // Call the CandidTypeRecord's decoder
    if (m_p_has_value) {
      *m_p_has_value = true;
    }
    offset_start = offset;
    parse_error = "";

    if (m_pv) {
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        B.debug_print_as_hex_string(debug_hex_string, offset_start, offset);
        ICPP_HOOKS::debug_print("Calling m_pv->decode_M");
      }
      if (m_pv->decode_M(de, B, offset, parse_error)) {
        std::string to_be_parsed =
            "Values (decoding M) for CandidTypeRecord of CandidTypeOptRecord ";
        CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                            parse_error);
      }
    } else {
      ICPP_HOOKS::trap("ERROR: m_pv in opt record is a nullptr.");
    }
  } else if (tag != 0) {
    ICPP_HOOKS::trap("ERROR: tag in opt record coming from wire is not 0 or 1");
  }
  return false;
}

//------------
// Initialize things
void CandidTypeOptRecord::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeOptRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// build the type table encoding
void CandidTypeOptRecord::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // The type-table-index of the Record that is Optional
  __uint128_t m_content_datatype = m_v.get_type_table_index();
  m_T.append_uleb128(m_content_datatype);

  // Update the type table registry
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// To decode the type table on the wire, starting at & updating offset
bool CandidTypeOptRecord::decode_T(VecBytes B, __uint128_t &offset,
                                   std::string &parse_error) {
  __uint128_t len = B.size() - offset;

  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type; // type-table index of the Record
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed =
        "Type table: a OptRecord's content type (the Record's type table index)";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }
  assert(content_type >= 0);
  m_content_datatype = int(content_type);
  return false;
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptRecord::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
