// Candid type table
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include <string>

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "candid_type_table.h"

#include "icpp_hooks.h"

#include <cassert>
#include <utility>

CandidTypeTable::CandidTypeTable() {}

CandidTypeTable::CandidTypeTable(const VecBytes &B, __uint128_t &B_offset) {
  m_B = B;
  m_B_offset = B_offset;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_hex_string_wire = m_B.as_hex_string();
  }
  deserialize(B_offset);
}

CandidTypeTable::~CandidTypeTable() {}

void CandidTypeTable::deserialize(__uint128_t &B_offset) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("---------------------------");
    ICPP_HOOKS::debug_print("Entered CandidTypeTable::deserialize");
    ICPP_HOOKS::debug_print("Start deserialization of a Type Table");
  }
  CandidOpcode candidOpcode;

  // Get the datatype for the type table
  __uint128_t B_offset_start = B_offset;
  std::string parse_error;
  __int128_t datatype;
  __uint128_t numbytes;
  if (m_B.parse_sleb128(B_offset, datatype, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: opcode";
    CandidAssert::trap_with_parse_error(B_offset_start, B_offset, to_be_parsed,
                                        parse_error);
  }
  m_datatype = int(datatype);
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start, B_offset);
    ICPP_HOOKS::debug_print(
        "datatype = " + ICPP_HOOKS::to_string_128(datatype) + " (" +
        candidOpcode.name_from_opcode(m_datatype) + ")");
  }

  // Deserialize the type-table
  // (-) Create a dummy CandidType and store it in m_p_wire
  // (-) Use m_p_wire.decode_T here to read the wire

  if (m_datatype < 0) {
    m_opcode = m_datatype;
    if (m_opcode != candidOpcode.Vec && m_opcode != candidOpcode.Opt) {
      // If not a Vec & Opt, we can simply create the dummy CandidType c
      m_p_wire = candidOpcode.candid_type_from_opcode(m_opcode);
      m_p_wire->set_is_internal(true);

      // We now have a dummy CandidType, and we can use decode_T
      B_offset_start = B_offset;
      parse_error = "";
      if (m_p_wire->decode_T(m_B, B_offset, parse_error)) {
        std::string to_be_parsed = "Parsing of the type tables.";
        CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                            to_be_parsed, parse_error);
      }
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start,
                                      B_offset);
        ICPP_HOOKS::debug_print(
            "Parsed the type table with decode_T of dummy CandidType instance");
      }
    } else {
      // for a Vec & Opt, we do not yet know what kind of vec or opt it is
      // and can not yet create the dummy m_p_wire.
      // It's type table is very simple though, just a content type.

      // Just read the content_datatype and then return
      // After all the type tables are read, we will create m_p_wire
      B_offset_start = B_offset;
      parse_error = "";
      __int128_t content_datatype;
      if (m_B.parse_sleb128(B_offset, content_datatype, numbytes,
                            parse_error)) {
        std::string to_be_parsed = "Type table: content type for a " +
                                   candidOpcode.name_from_opcode(m_opcode);
        CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                            to_be_parsed, parse_error);
      }
      m_content_datatype = int(content_datatype);
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start,
                                      B_offset);
        ICPP_HOOKS::debug_print(
            "Manually parsed type table for " +
            candidOpcode.name_from_opcode(m_opcode) +
            "\nThe content_datatype found = " +
            std::to_string(m_content_datatype) + " (" +
            candidOpcode.name_from_opcode(m_content_datatype) + ")");
      }
    }
  }
}

// Once a proper content_opcode found, set m_content_opcode and create a dummy m_p_wire
void CandidTypeTable::finish_vec_and_opt(
    int content_opcode, CandidTypeTable *p_content_type_table) {
  CandidOpcode candidOpcode;
  if (m_opcode != candidOpcode.Vec && m_opcode != candidOpcode.Opt) {
    ICPP_HOOKS::trap(std::string(__func__) +
                     ": ERROR - this is not a Vec or Opt !");
  }

  m_content_opcode = content_opcode;

  if (m_opcode == candidOpcode.Vec) {
    m_p_wire = candidOpcode.candid_type_vec_from_opcode(content_opcode,
                                                        p_content_type_table);
    m_p_wire->set_content_datatype(m_content_datatype);
  } else if (m_opcode == candidOpcode.Opt) {
    m_p_wire = candidOpcode.candid_type_opt_from_opcode(content_opcode,
                                                        p_content_type_table);
    m_p_wire->set_content_datatype(m_content_datatype);
  } else {
    std::string msg;
    msg.append(std::string(__func__) + ": ERROR: internal code error");
    msg.append("\n- m_opcode = " + std::to_string(m_opcode));
    msg.append("\n- content_opcode = " + std::to_string(content_opcode));
    ICPP_HOOKS::trap(msg);
  }
  if (m_p_wire) m_p_wire->set_is_internal(true);
}