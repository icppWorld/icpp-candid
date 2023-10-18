// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid_debug_config.h"
#include "candid_deserialize.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "icpp_hooks.h"

CandidDeserialize::CandidDeserialize() { deserialize(); }
CandidDeserialize::CandidDeserialize(const VecBytes &B, CandidArgs A) {
  m_A = A;
  m_B = B;
  m_hex_string = ""; // TOdo
  deserialize();
}
CandidDeserialize::CandidDeserialize(const std::string hex_string,
                                     CandidArgs A) {
  m_A = A;
  m_hex_string = hex_string;
  m_B.store_hex_string(hex_string);
  deserialize();
}
CandidDeserialize::~CandidDeserialize() {}

void CandidDeserialize::deserialize() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#deserialisation
  // Deserialisation at an expected type sequence (<t'>,*) proceeds by
  //
  // (1) checking for the magic number DIDL
  // (2) using the inverse of the T function to decode the type definitions (<t>,*)
  // (3) check that (<t>,*) <: (<t'>,*), else fail
  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  //

  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results
  // B(kv* : <datatype>*) =
  // i8('D') i8('I') i8('D') i8('L')      magic number
  // T*(<comptype>*)                      type definition table
  // I*(<datatype>*)                      types of the argument list
  // M(kv* : <datatype>*)                 values of argument list

  CandidOpcode candidOpcode;
  __uint128_t B_offset = 0;

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("---------------------------");
    ICPP_HOOKS::debug_print("Entered CandidDeserialize::deserialize");
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
  }

  // -------------------------------------------------------------------------------------
  // (1) checking for the magic number DIDL
  m_B.trap_if_vec_does_not_start_with_DIDL();
  B_offset = 4;

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("Found DIDL");
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
  }

  // -------------------------------------------------------------------------------------
  // (2) using the inverse of the T function to decode the type definitions (<t>,*)
  //
  // T*(<comptype>*)                      type definition table
  // Write the unique TypeTables the comptypes:
  // (-) Constructed Types (opt, vec, record, variant)
  // (-) Reference Types (func, service)

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "********************** T ************************");
  }
  // Parse the number of unique type tables
  __uint128_t num_typetables;

  {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_typetables, numbytes, parse_error)) {
      std::string to_be_parsed =
          "Number of unique type tables, T*(<comptype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("num_typetables = " +
                            ICPP_HOOKS::to_string_128(num_typetables));
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
  }

  // Parse all the type tables
  for (size_t i = 0; i < num_typetables; ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print("--");
      ICPP_HOOKS::debug_print("Parsing type table index " + std::to_string(i));
      ICPP_HOOKS::debug_print("B_offset = " +
                              ICPP_HOOKS::to_string_128(B_offset));
    }
    CandidTypeTable type_table = CandidTypeTable(m_B, B_offset);
    m_typetables.push_back(type_table);
  }

  // Finish type-table settings for Opt & Vec
  for (size_t i = 0; i < num_typetables; ++i) {
    if (m_typetables[i].get_opcode() == candidOpcode.Vec ||
        m_typetables[i].get_opcode() == candidOpcode.Opt) {

      int content_opcode = m_typetables[i].get_content_opcode();

      // Try at most num_typetables times to get a negative content_opcode
      int tries = 0;
      while (content_opcode >= 0) {
        // This is an index into the type tables, so get the opcode of that type-table
        if (content_opcode < m_typetables.size()) {
          content_opcode = m_typetables[content_opcode].get_opcode();
        }
        tries++;
        if (tries >= num_typetables) {
          break; // break out of the loop after num_typetables tries
        }
      }

      if (content_opcode >= 0) {
        ICPP_HOOKS::trap(
            std::string(__func__) +
            ": Cannot find the actual content type in the type tables for a Vec or Opt.");
      }

      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print("calling m_c_for_vec_and_opt for type_table " +
                                std::to_string(i));
      }
      m_typetables[i].set_vec_and_opt(content_opcode);
    }
  }

  // -------------------------------------------------------------------------------------
  // I*(<datatype>*)                      types of the argument list
  //                                      - Opcode for <primtype>
  //                                      or
  //                                      - TypeTable index for <comptype>

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "\n********************** I ************************");
  }
  m_args_datatypes.clear();
  m_args_datatypes_offset_start.clear();
  m_args_datatypes_offset_end.clear();

  // Parse the number of arguments in the byte stream
  __uint128_t num_args;

  {
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_args, numbytes, parse_error)) {
      std::string to_be_parsed = "Number of arguments, I*(<datatype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("num_args = " +
                            ICPP_HOOKS::to_string_128(num_args));
    ICPP_HOOKS::debug_print("B_offset = " +
                            ICPP_HOOKS::to_string_128(B_offset));
  }

  if (num_args != m_A.m_args_ptrs.size()) {
    std::string msg;
    msg.append("ERROR: wrong number of arguments on wire.\n");
    msg.append("       Expected number of arguments:" +
               std::to_string(m_A.m_args_ptrs.size()) + "\n");
    msg.append("       Number of arguments on wire :" +
               ICPP_HOOKS::to_string_128(num_args));
    ICPP_HOOKS::trap(msg);
  }

  for (size_t i = 0; i < num_args; ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print("--\narg = " + std::to_string(i));
    }
    __uint128_t B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    __int128_t datatype;
    if (m_B.parse_sleb128(B_offset, datatype, numbytes, parse_error)) {
      std::string to_be_parsed = "datatype, I*(<datatype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "datatype = " + ICPP_HOOKS::to_string_128(datatype) + "(" +
          candidOpcode.name_from_opcode(datatype) + ")");
      ICPP_HOOKS::debug_print("B_offset = " +
                              ICPP_HOOKS::to_string_128(B_offset));
    }
    m_args_datatypes.push_back(int(datatype));
    m_args_datatypes_offset_start.push_back(B_offset_start);
    m_args_datatypes_offset_end.push_back(B_offset);
  }

  // -------------------------------------------------------------------------------------
  // (3) check that (<t>,*) <: (<t'>,*), else fail
  //
  //  (<t>,*)  = found type definitions
  //  (<t'>,*) = expected type definitions, provided via m_pA
  //  <:       = the found type definitions (<t>,*) are structural subtypes of the expected type sequence (<t'>,*)
  //
  //             https://github.com/dfinity/candid/blob/master/spec/Candid.md#upgrading-and-subtyping
  //
  check_types();

  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  //
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  // M(kv* : <datatype>*)                 values of argument list

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "\n********************** M ************************");
  }
  for (size_t i = 0; i < num_args; ++i) {
    if (m_args_datatypes[i] == candidOpcode.Null) {
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "The opcode was Null, so nothing to read & decode for arg " +
            std::to_string(i));
      }
      // There is no value to decode
      continue;
    }

    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg;
      msg.append("Start reading data by calling decode_M for arg " +
                 std::to_string(i));
      msg.append(
          "\n- Using decode_M of expected datatype : m_A.m_args_ptrs[i]  = " +
          std::to_string(m_A.m_args_ptrs[i]->get_datatype_opcode()) + " (" +
          m_A.m_args_ptrs[i]->get_datatype_textual() + ")");
      ICPP_HOOKS::debug_print(msg);
    }
    __uint128_t B_offset_start = B_offset;
    std::string parse_error = "";
    __uint128_t numbytes;

    if (m_A.m_args_ptrs[i]->decode_M(m_B, B_offset, parse_error)) {
      std::string to_be_parsed =
          "Values (decoding M) for argument at index " + std::to_string(i);
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print("Done reading data by calling decode_M for arg " +
                              std::to_string(i));
      ICPP_HOOKS::debug_print("B_offset = " +
                              ICPP_HOOKS::to_string_128(B_offset));
    }
  }

  // Append R
  // Was never implemented int Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

void CandidDeserialize::check_types() {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("--------------------------------");
    ICPP_HOOKS::debug_print(
        "Check all the found datatypes against expected (JUST A CHECK, NO READING FROM WIRE...)");
  }
  CandidOpcode candidOpcode;
  for (size_t i = 0; i < m_args_datatypes.size(); ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print("--\narg = " + std::to_string(i));
    }

    if (m_args_datatypes[i] < 0) {
      // <primtype>, because we found an Opcode, not a type table
      int opcode_found = m_args_datatypes[i];
      int opcode_expected = m_A.m_args_ptrs[i]->get_datatype_opcode();

      if (opcode_found != opcode_expected) {
        if (opcode_found == candidOpcode.Null &&
            opcode_expected == candidOpcode.Opt) {
          // This is ok. A null is passed for an Opt
        } else {
          // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
          std::string msg;
          msg.append("ERROR: wrong opcode found on wire.\n");
          msg.append("       Argument index: " + std::to_string(i) + "\n");
          msg.append(
              "       Bytes offset start: " +
              ICPP_HOOKS::to_string_128(m_args_datatypes_offset_start[i]) +
              "\n");
          msg.append("       Bytes offset end  : " +
                     ICPP_HOOKS::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append(
              "       Expecting opcode:" + std::to_string(opcode_expected) +
              " (" + candidOpcode.name_from_opcode(opcode_expected) + ")" +
              "\n");
          msg.append("       Found opcode    :" + std::to_string(opcode_found) +
                     " (" + candidOpcode.name_from_opcode(opcode_found) + ")");
          ICPP_HOOKS::trap(msg);
        }
      }
    } else {
      // <comptype>, because we found a type-table

      // the datatype is an index into the typetable of the byte stream
      size_t type_table_index = m_args_datatypes[i];
      CandidTypeTable type_table = m_typetables[type_table_index];

      // Verify that the datatype or type-table-content found on the wire is the same as the expected
      // Note: the type-table-index itself can be completely different...
      int opcode_found = type_table.get_opcode();
      int opcode_expected = m_A.m_args_ptrs[i]->get_datatype_opcode();

      if (opcode_expected < 0) {
        if (opcode_found < 0) {
          // Both are actual candid types, and not type-tables
          if (opcode_found == opcode_expected) {
            // Ok, found two of the same constype
            // Verify if their type-tables are the same
            if (opcode_found == candidOpcode.Record) {
              CandidTypeRecord *p_wire =
                  get_if<CandidTypeRecord>(type_table.get_candidType_ptr());
              std::shared_ptr<CandidTypeRecord> p_expected =
                  std::dynamic_pointer_cast<CandidTypeRecord>(
                      m_A.m_args_ptrs[i]);
              if (!p_expected) {
                ICPP_HOOKS::trap(
                    "ERROR: Expecting a CandidTypeRecord during deserialization.");
              } else if (p_wire) {
                // Traps if type table of Record does not match the wire
                p_expected->check_type_table(p_wire);
              } else {
                ICPP_HOOKS::trap(
                    "ERROR: Logic error during deserialization of a CandidTypeRecord.");
              }
            } else if (opcode_found == candidOpcode.Variant) {
              CandidTypeVariant *p_wire =
                  get_if<CandidTypeVariant>(type_table.get_candidType_ptr());
              std::shared_ptr<CandidTypeVariant> p_expected =
                  std::dynamic_pointer_cast<CandidTypeVariant>(
                      m_A.m_args_ptrs[i]);
              if (!p_expected) {
                ICPP_HOOKS::trap(
                    "ERROR: Expecting a CandidTypeVariant during deserialization.");
              } else if (p_wire) {
                // Traps if type table of Variant does not match the wire
                p_expected->check_type_table(p_wire);
              } else {
                ICPP_HOOKS::trap(
                    "ERROR: Logic error during deserialization of a CandidTypeVariant.");
              }
            } else if (opcode_found == candidOpcode.Vec ||
                       opcode_found == candidOpcode.Opt) {

              int content_opcode_found = type_table.get_content_opcode();
              int content_opcode_expected =
                  m_A.m_args_ptrs[i]->get_content_type_opcode();

              if (content_opcode_found != content_opcode_expected) {
                std::string msg;
                if (opcode_found == candidOpcode.Vec) {
                  msg.append(
                      "ERROR: Vector with wrong content opcode found on wire.\n");
                } else if (opcode_found == candidOpcode.Opt) {
                  msg.append(
                      "ERROR: Opt with wrong content opcode found on wire.\n");
                } else {
                  msg.append(
                      "ERROR: const of unknown type with wrong content opcode found on wire.\n");
                }

                msg.append("       Argument index: " + std::to_string(i) +
                           "\n");
                msg.append("       Bytes offset start: " +
                           ICPP_HOOKS::to_string_128(
                               m_args_datatypes_offset_start[i]) +
                           "\n");
                msg.append(
                    "       Bytes offset end  : " +
                    ICPP_HOOKS::to_string_128(m_args_datatypes_offset_end[i]) +
                    "\n");
                msg.append(
                    "       Expecting content opcode:" +
                    std::to_string(content_opcode_expected) + " (" +
                    candidOpcode.name_from_opcode(content_opcode_expected) +
                    ")" + "\n");
                msg.append("       Found content opcode    :" +
                           std::to_string(content_opcode_found) + " (" +
                           candidOpcode.name_from_opcode(content_opcode_found) +
                           ")" + "\n");
                ICPP_HOOKS::trap(msg);
              }
            } else {
              ICPP_HOOKS::trap(
                  "ERROR: Deserialization not yet implemented for this constype");
            }

            continue; // All good
          } else {
            // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
            std::string msg;
            msg.append("ERROR: Wrong opcode found on wire.\n");
            msg.append("       Argument index: " + std::to_string(i) + "\n");
            msg.append(
                "       Bytes offset start: " +
                ICPP_HOOKS::to_string_128(m_args_datatypes_offset_start[i]) +
                "\n");
            msg.append(
                "       Bytes offset end  : " +
                ICPP_HOOKS::to_string_128(m_args_datatypes_offset_end[i]) +
                "\n");
            msg.append(
                "       Expecting opcode:" + std::to_string(opcode_expected) +
                " (" + candidOpcode.name_from_opcode(opcode_expected) + ")" +
                "\n");
            msg.append(
                "       Found opcode    :" + std::to_string(opcode_found) +
                " (" + candidOpcode.name_from_opcode(opcode_found) + ")");
            ICPP_HOOKS::trap(msg);
          }
        } else {
          // opcode_expected is a candid type, but found a type-table that references a type-table
          // TODO: this could be ok...?
          std::string msg;
          msg.append(
              "ERROR: expecting a type-table with an Opcode, but a type table with a type table reference was found on wire instead.\n");
          msg.append("       Argument index    : " + std::to_string(i) + "\n");
          msg.append(
              "       Bytes offset start: " +
              ICPP_HOOKS::to_string_128(m_args_datatypes_offset_start[i]) +
              "\n");
          msg.append("       Bytes offset end  : " +
                     ICPP_HOOKS::to_string_128(m_args_datatypes_offset_end[i]) +
                     "\n");
          msg.append(
              "       Expecting opcode  :" + std::to_string(opcode_expected) +
              " (" + candidOpcode.name_from_opcode(opcode_expected) + ")" +
              "\n");
          msg.append("       Found type table    :" +
                     std::to_string(opcode_found));
          ICPP_HOOKS::trap(msg);
        }
      }
    }
  }
}

// Assert candid VecBytes against a string in "hex" format (didc encode)
int CandidDeserialize::assert_candid(const std::string &candid_expected,
                                     const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

CandidArgs CandidDeserialize::get_A() { return m_A; }
VecBytes CandidDeserialize::get_B() { return m_B; }