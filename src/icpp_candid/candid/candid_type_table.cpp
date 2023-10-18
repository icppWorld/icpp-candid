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
  deserialize(B_offset);
}

CandidTypeTable::~CandidTypeTable() {}

void CandidTypeTable::deserialize(__uint128_t &B_offset) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("---------------------------");
    ICPP_HOOKS::debug_print("Entered CandidTypeTable::deserialize");
    ICPP_HOOKS::debug_print("Start deserialization of a Type Table");
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
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
  m_opcode = int(datatype);
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "datatype = " + ICPP_HOOKS::to_string_128(datatype) + " (" +
        candidOpcode.name_from_opcode(m_opcode) + ")");
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
  }

  // Deserialize the type-table
  // (-) Create a dummy CandidType and store it in m_c
  // (-) Use m_c.decode_T here to read the wire
  // (-) We use m_c later in CandidTypeDeserialize::check_types()

  if (m_opcode != candidOpcode.Vec && m_opcode != candidOpcode.Opt) {
    // If not a Vec & Opt, we can simply create the dummy CandidType m_c
    candidOpcode.candid_type_from_opcode(m_c, m_opcode);
    // We now have a dummy CandidType m_c, and we can use decode_T
    parse_error = "";
    if (std::visit(
            [&](auto &&c) { return c.decode_T(m_B, B_offset, parse_error); },
            m_c)) {
      std::string to_be_parsed = "Parsing of the type tables.";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "Parsed the type table with decode_T of dummy CandidTypeTable::m_c");
      ICPP_HOOKS::debug_print("B_offset = " +
                              ICPP_HOOKS::to_string_128(B_offset));
    }
  } else {
    // for a Vec & Opt, we do not yet know what kind of vec or opt it is
    // and can not yet create the dummy m_c.
    // It's type table is very simple though, just a content type.

    // Just read the m_content_opcode and then return
    // After all the type tables are read, we will create m_c
    B_offset_start = B_offset;
    parse_error = "";
    __int128_t content_opcode;
    if (m_B.parse_sleb128(B_offset, content_opcode, numbytes, parse_error)) {
      std::string to_be_parsed = "Type table: content type for a " +
                                 candidOpcode.name_from_opcode(m_opcode);
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
    m_content_opcode = int(content_opcode);
    if (m_content_opcode >= 0) {
      m_content_table_index = m_content_opcode;
    }
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "Manually parsed type table for " +
          candidOpcode.name_from_opcode(m_opcode) +
          "\nThe content_opcode found = " + std::to_string(m_content_opcode) +
          " (" + candidOpcode.name_from_opcode(m_content_opcode) + ")");
      ICPP_HOOKS::debug_print("B_offset = " +
                              ICPP_HOOKS::to_string_128(B_offset));
    }
  }
}

// Once a proper content_opcode found, set m_content_opcode and create a dummy m_c
void CandidTypeTable::set_vec_and_opt(int content_opcode) {
  CandidOpcode candidOpcode;
  if (m_opcode != candidOpcode.Vec && m_opcode != candidOpcode.Opt) {
    ICPP_HOOKS::trap(std::string(__func__) +
                     ": ERROR - this is not a Vec or Opt !");
  }

  m_content_opcode = content_opcode;

  if (m_opcode == candidOpcode.Vec) {
    candidOpcode.candid_type_vec_from_opcode(m_c, content_opcode);
  } else if (m_opcode == candidOpcode.Opt) {
    candidOpcode.candid_type_opt_from_opcode(m_c, content_opcode);
  } else {
    std::string msg;
    msg.append(std::string(__func__) + ": ERROR: internal code error");
    msg.append("\n- m_opcode = " + std::to_string(m_opcode));
    msg.append("\n- content_opcode = " + std::to_string(content_opcode));
    ICPP_HOOKS::trap(msg);
  }
}