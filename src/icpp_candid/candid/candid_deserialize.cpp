// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include "candid_debug_config.h"
#include "candid_constants.h"
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
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_hex_string_wire = m_B.as_hex_string();
  }
  deserialize();
}
CandidDeserialize::CandidDeserialize(const std::string hex_string,
                                     CandidArgs A) {
  m_A = A;
  m_hex_string_wire = hex_string;
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
  __uint128_t B_offset_start = 0;
  __uint128_t B_offset = 0;

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("---------------------------");
    ICPP_HOOKS::debug_print("Entered CandidDeserialize::deserialize");
  }

  // -------------------------------------------------------------------------------------
  // (1) checking for the magic number DIDL
  m_B.trap_if_vec_does_not_start_with_DIDL();
  B_offset = 4;

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start, B_offset);
    ICPP_HOOKS::debug_print("Found DIDL");
  }

  // -------------------------------------------------------------------------------------
  // (2) using the inverse of the T function to decode the type definitions (<t>,*)
  //
  // T*(<comptype>*)                      type definition table
  // Write the unique TypeTables the comptypes:
  // (-) Constructed Types (opt, vec, record, variant)
  // (-) Reference Types (func, service)

  // Notes:
  // (-) Section T can contain additional, unused type tables
  // (-) The order is totally random, unrelated to the order of the args on the wire
  // (-) What is actually on the wire and in what order is defined in section I

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "********************** T ************************");
  }
  // Parse the number of unique type tables
  __uint128_t num_typetables_wire;

  {
    B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_typetables_wire, numbytes,
                          parse_error)) {
      std::string to_be_parsed =
          "Number of unique type tables, T*(<comptype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start, B_offset);
    ICPP_HOOKS::debug_print("num_typetables_wire = " +
                            ICPP_HOOKS::to_string_128(num_typetables_wire));
  }

  // Parse all the type tables
  for (size_t i = 0; i < num_typetables_wire; ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
      ICPP_HOOKS::debug_print("Start parsing of type table index " +
                              std::to_string(i));
    }
    B_offset_start = B_offset;
    CandidTypeTable type_table = CandidTypeTable(m_B, B_offset);
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start,
                                    B_offset);
      ICPP_HOOKS::debug_print("Parsed type table index " + std::to_string(i));
    }
    m_typetables_wire.push_back(type_table);
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  }

  // Set the opcode for type-tables that refer to a type-table
  for (size_t i = 0; i < num_typetables_wire; ++i) {
    int datatype_wire = m_typetables_wire[i].get_datatype();
    if (datatype_wire >= 0) {
      int opcode_wire = get_opcode_from_datatype_on_wire(datatype_wire);
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "Setting opcode, using opcode_wire, for type table index " +
            std::to_string(i));
      }
      m_typetables_wire[i].set_opcode(opcode_wire);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  }

  // Finish creation of decoders for the type-tables of type Opt & Vec
  // We need to make multiple loops
  int num_to_do_previous{0};
  while (true) {
    int num_to_do{0};
    for (size_t i = 0; i < num_typetables_wire; ++i) {
      if (m_typetables_wire[i].get_opcode() == candidOpcode.Vec ||
          m_typetables_wire[i].get_opcode() == candidOpcode.Opt) {

        int content_opcode_wire;

        int content_datatype = m_typetables_wire[i].get_content_datatype();
        CandidTypeTable *p_content_type_table = nullptr;
        if (content_datatype >= 0) {
          if (m_typetables_wire[content_datatype].get_p_wire()) {
            p_content_type_table = &m_typetables_wire[content_datatype];
            content_opcode_wire =
                p_content_type_table->get_p_wire()->get_datatype_opcode();
            assert(content_opcode_wire ==
                   get_opcode_from_datatype_on_wire(content_datatype));
          } else {
            ++num_to_do;
            continue;
          }
        } else {
          content_opcode_wire = content_datatype;
        }
        if (CANDID_DESERIALIZE_DEBUG_PRINT) {
          ICPP_HOOKS::debug_print(
              "Calling finish_vec_and_opt for type table index " +
              std::to_string(i) + " passing content_opcode_wire = " +
              std::to_string(content_opcode_wire) + " (" +
              candidOpcode.name_from_opcode(content_opcode_wire) + ")");
        }
        m_typetables_wire[i].finish_vec_and_opt(content_opcode_wire,
                                                p_content_type_table);
      }
    }
    if (num_to_do == 0) {
      break;
    }
    if (num_to_do > 0) {
      if (num_to_do_previous != 0 && num_to_do >= num_to_do_previous) {
        // Nothing got set during this loop. Something is wrong.
        ICPP_HOOKS::trap(
            "ERROR: do not understand the Type Table section on the wire.");
      }
      // We still got unset items. Let's try again
      num_to_do_previous = num_to_do;
      num_to_do = 0;
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  }

  // Finish processing the type-table info for Record & Variant
  for (size_t i = 0; i < num_typetables_wire; ++i) {
    if ((m_typetables_wire[i].get_opcode() == candidOpcode.Record) ||
        (m_typetables_wire[i].get_opcode() == candidOpcode.Variant)) {
      if (CANDID_DESERIALIZE_DEBUG_PRINT) {
        ICPP_HOOKS::debug_print(
            "Calling finish_decode_T for type table index " +
            std::to_string(i));
      }
      m_typetables_wire[i].get_p_wire()->finish_decode_T(*this);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  }

  // -------------------------------------------------------------------------------------
  // I*(<datatype>*)                      types of the argument list
  //                                      - Opcode for <primtype>
  //                                      or
  //                                      - TypeTable index for <comptype>

  // Notes:
  // (-) Section I contains what is actually on the wire in section M and in what order

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "\n********************** I ************************");
  }
  m_args_datatypes_wire.clear();
  m_args_opcodes_wire.clear();
  m_args_content_datatypes_wire.clear();
  m_args_content_opcodes_wire.clear();
  m_args_datatypes_offset_start_wire.clear();
  m_args_datatypes_offset_end_wire.clear();

  // Parse the number of arguments in the byte stream
  __uint128_t num_args_wire;

  {
    B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    if (m_B.parse_uleb128(B_offset, num_args_wire, numbytes, parse_error)) {
      std::string to_be_parsed = "Number of arguments on wire, I*(<datatype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }
  }
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start, B_offset);
    ICPP_HOOKS::debug_print("num_args_wire = " +
                            ICPP_HOOKS::to_string_128(num_args_wire));
  }

  // Parse the data_type
  for (size_t i = 0; i < num_args_wire; ++i) {
    B_offset_start = B_offset;
    std::string parse_error;
    __uint128_t numbytes;
    __int128_t datatype_wire;
    if (m_B.parse_sleb128(B_offset, datatype_wire, numbytes, parse_error)) {
      std::string to_be_parsed = "datatype, I*(<datatype>*)";
      CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                          to_be_parsed, parse_error);
    }

    // Fill the references
    int opcode_wire = int(datatype_wire);
    int content_datatype_wire{CANDID_UNDEF};
    int content_opcode_wire{CANDID_UNDEF};
    if (opcode_wire >= 0) {
      int idx = opcode_wire;
      opcode_wire = m_typetables_wire[idx].get_opcode();
      content_datatype_wire = m_typetables_wire[idx].get_content_datatype();
      content_opcode_wire = m_typetables_wire[idx].get_content_opcode();
    }

    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start,
                                    B_offset);
      std::string msg;
      msg.append("arg = " + std::to_string(i));
      msg.append("\n - datatype_wire = " +
                 ICPP_HOOKS::to_string_128(datatype_wire));
      msg.append("\n - opcode_wire   = " + std::to_string(opcode_wire) + " (" +
                 candidOpcode.name_from_opcode(opcode_wire) + ")");
      if (content_datatype_wire != CANDID_UNDEF) {
        msg.append("\n - content_datatype_wire = " +
                   std::to_string(content_datatype_wire));
      }
      if (content_opcode_wire != CANDID_UNDEF) {
        msg.append("\n - content_opcode_wire = " +
                   std::to_string(content_opcode_wire) + " (" +
                   candidOpcode.name_from_opcode(content_opcode_wire) + ")");
      }
      ICPP_HOOKS::debug_print(msg);
    }
    m_args_datatypes_wire.push_back(int(datatype_wire));
    m_args_opcodes_wire.push_back(opcode_wire);
    m_args_content_datatypes_wire.push_back(content_datatype_wire);
    m_args_content_opcodes_wire.push_back(content_opcode_wire);
    m_args_datatypes_offset_start_wire.push_back(B_offset_start);
    m_args_datatypes_offset_end_wire.push_back(B_offset);
  }

  // -------------------------------------------------------------------------------------
  // (3) check that (<t>,*) <: (<t'>,*), else fail
  //
  //  (<t>,*)  = found type definitions in section I, stored in m_args_datatypes_wire
  //  (<t'>,*) = expected type definitions, provided via m_pA
  //  <:       = the found type definitions (<t>,*) are structural subtypes of the expected type sequence (<t'>,*)
  //
  //             https://github.com/dfinity/candid/blob/master/spec/Candid.md#upgrading-and-subtyping
  //

  // (4) using the inverse of the M function, indexed by (<t>,*), to decode the values (<v>,*)
  //
  // (5) use the coercion function C[(<t>,*) <: (<t'>,*)]((<v>,*)) to understand the decoded values at the expected type.
  // M(kv* : <datatype>*)                 values of argument list
  //
  // Notes:
  // (-) A Candid explainer: Opt is special
  //     https://www.joachim-breitner.de/blog/784-A_Candid_explainer__Opt_is_special
  //     -> additional Opt in the args on wire must be parsed & discarded
  //     -> missing Opt on the wire is Ok, it is an Opt after all

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print(
        "\n********************** M ************************");
  }

  if (m_A.m_args_ptrs.size() > 0) {
    // We have something to decode...

    // index into found args on wire
    size_t j_now = 0;

    bool handled_arg_expected{false};
    //                   <= to check that additional args on wire are Opts, else trap
    for (size_t i = 0; i <= m_A.m_args_ptrs.size(); ++i) {
      if (i < m_A.m_args_ptrs.size()) {
        handled_arg_expected = false;
      } else {
        // We're already done with all expected args.
        // Just checking for additional Opt args, which is allowed
        // It will trap if additional non Opt args are found
        handled_arg_expected = true;
      }

      if (j_now == m_args_datatypes_wire.size()) {
        if (i < m_A.m_args_ptrs.size()) {
          int opcode_expected = m_A.m_args_ptrs[i]->get_datatype_opcode();
          if (opcode_expected == candidOpcode.Opt) {
            // OK! We have an Opt as next argument and nothing left on the wire
            handled_arg_expected = true;
          }
        }
      } else {
        for (size_t j = j_now; j < m_args_datatypes_wire.size(); ++j) {

          // Check if all is OK and if there is anything to decode, with either expected or wire decoder
          std::shared_ptr<CandidTypeRoot> decoder = nullptr;
          std::string decoder_name;

          select_decoder_or_trap(i, j, decoder, decoder_name);

          if (decoder) {
            if (CANDID_DESERIALIZE_DEBUG_PRINT) {
              std::string msg;
              msg.append("Start reading data by calling decode_M for arg " +
                         std::to_string(i));
              msg.append("\n- Using decode_M of " + decoder_name +
                         " for Opcode " +
                         std::to_string(decoder->get_datatype_opcode()) + " (" +
                         decoder->get_datatype_textual() + ")");
              ICPP_HOOKS::debug_print(msg);
            }
            B_offset_start = B_offset;
            std::string parse_error = "";
            __uint128_t numbytes;

            if (decoder->decode_M(*this, m_B, B_offset, parse_error)) {
              std::string to_be_parsed =
                  "Values (decoding M) for argument at index " +
                  std::to_string(i);
              CandidAssert::trap_with_parse_error(B_offset_start, B_offset,
                                                  to_be_parsed, parse_error);
            }

            if (CANDID_DESERIALIZE_DEBUG_PRINT) {
              m_B.debug_print_as_hex_string(m_hex_string_wire, B_offset_start,
                                            B_offset);
              ICPP_HOOKS::debug_print(
                  "Done reading data by calling decode_M for arg " +
                  std::to_string(i));
            }
          }

          if (decoder_name == "expected" ||
              decoder_name == "expected-opt-as-null" ||
              decoder_name == "expected-opt-as-opt-null") {
            // Found Expected on the wire
            ++j_now;
            handled_arg_expected = true;
            break;
          } else if (decoder_name == "skip-expected-opt-not-found-on-wire") {
            // Expected Opt not found on the wire, but that is OK
            // -> keep j where it is, to check if next expected arg matches
            handled_arg_expected = true;
            break;
          } else if (decoder_name == "read-and-discard-additional-wire-opt") {
            // Additional Opt on the wire, is also OK, just check next on wire
            ++j_now;
          } else {
            std::string msg;
            msg.append("ERROR: invalid decoder_name.");
            ICPP_HOOKS::trap(msg);
          }
        }
      }

      if (!handled_arg_expected) {
        std::string msg;
        msg.append("\nERROR: Did not receive expected arg at index " +
                   std::to_string(i));
        if (i < m_A.m_args_ptrs.size()) {
          int opcode_expected = m_A.m_args_ptrs[i]->get_datatype_opcode();
          msg.append("\n- Opcode   = " + std::to_string(opcode_expected) +
                     " (" + candidOpcode.name_from_opcode(opcode_expected) +
                     ")");
        }
        ICPP_HOOKS::trap(msg);
      }
    }
  }

  // Append R
  // Was never implemented int Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

void CandidDeserialize::select_decoder_or_trap(
    size_t i, size_t j, std::shared_ptr<CandidTypeRoot> &decoder,
    std::string &decoder_name) {

  CandidOpcode candidOpcode;

  // The arg on the wire (j)
  int datatype_wire = m_args_datatypes_wire[j];
  int opcode_wire = m_args_opcodes_wire[j];
  int content_opcode_wire{CANDID_UNDEF};
  std::string content_name_wire;
  if (opcode_wire == candidOpcode.Opt || opcode_wire == candidOpcode.Vec) {
    // datatype_wire is an index into the type-tables
    int idx = datatype_wire;
    if (idx >= 0 && idx < m_typetables_wire.size()) {
      content_opcode_wire = m_typetables_wire[idx].get_content_opcode();
      content_name_wire = candidOpcode.name_from_opcode(content_opcode_wire);
    } else {
      ICPP_HOOKS::trap("ERROR: Logic error with retrieval of Opt. or Vec");
    }
  }

  if (i >= m_A.m_args_ptrs.size()) {
    // We're done with the expected args, but we need to trap on possible additional non Opts on wire at end
    if (opcode_wire != candidOpcode.Opt) {
      std::string msg;
      msg.append(
          "\nERROR: Done with all the expected args, but there is an additional non Opt arg on the wire.");
      msg.append("\nnext arg on wire at index " + std::to_string(j));
      msg.append("\n- datatype     = " + std::to_string(datatype_wire));
      msg.append("\n- Opcode       = " + std::to_string(opcode_wire) + " (" +
                 candidOpcode.name_from_opcode(opcode_wire) + ")");
      if (datatype_wire >= 0) {
        msg.append("\n  (walked the type-tables to find that Opcode!)");
      }
      if (opcode_wire == candidOpcode.Opt) {
        msg.append("\n- opt_content_opcode_wire = " +
                   std::to_string(content_opcode_wire) + " (" +
                   content_name_wire + ")");
      }
      ICPP_HOOKS::trap(msg);
    }

    // OK, the next field on the wire is an additional Opt at the end
    // Additional Opt on wire must be decoded and discarded
    decoder = build_decoder_wire_for_additional_opt_arg(j);
    decoder_name = "read-and-discard-additional-wire-opt";

    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg = "--";
      msg.append(
          "\nDone with all the expected args, but there is an additional Opt arg on the wire...");

      msg.append("\nnext field on wire at index " + std::to_string(j));
      msg.append("\n- datatype     = " + std::to_string(datatype_wire));
      msg.append("\n- Opcode       = " + std::to_string(opcode_wire) + " (" +
                 candidOpcode.name_from_opcode(opcode_wire) + ")");
      if (datatype_wire >= 0) {
        msg.append("\n  (walked the type-tables to find that Opcode!)");
      }
      if (opcode_wire == candidOpcode.Opt) {
        msg.append("\n- opt_content_opcode_wire = " +
                   std::to_string(content_opcode_wire) + " (" +
                   content_name_wire + ")");
      }
      msg.append("\n--> We will decode M & discard the data.");
      ICPP_HOOKS::debug_print(msg);
    }
    return;
  }

  // We are still looking for expected args
  // The expected arg (i)
  int opcode_expected = m_A.m_args_ptrs[i]->get_datatype_opcode();
  int content_opcode_expected{CANDID_UNDEF};
  std::string content_name_expected;
  if (opcode_expected == candidOpcode.Opt ||
      opcode_expected == candidOpcode.Vec) {
    content_opcode_expected = m_A.m_args_ptrs[i]->get_content_opcode();
    content_name_expected =
        candidOpcode.name_from_opcode(content_opcode_expected);
  }
  // Sanity check: The encoded arg datatype during serialization is always the negative Opcode of the expected arg
  assert(opcode_expected < 0);

  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    std::string msg = "--";
    msg.append("\nexpected arg at index " + std::to_string(i));
    msg.append("\n- Opcode   = " + std::to_string(opcode_expected) + " (" +
               candidOpcode.name_from_opcode(opcode_expected) + ")");
    if (opcode_expected == candidOpcode.Opt) {
      msg.append("\n- content_opcode_expected = " +
                 std::to_string(content_opcode_expected) + " (" +
                 content_name_expected + ")");
    }

    msg.append("\nnext arg on wire at index " + std::to_string(j));
    msg.append("\n- datatype = " + std::to_string(datatype_wire));
    msg.append("\n- Opcode   = " + std::to_string(opcode_wire) + " (" +
               candidOpcode.name_from_opcode(opcode_wire) + ")");
    if (datatype_wire >= 0) {
      msg.append("\n  (walked the type-tables to find that Opcode!)");
    }
    if (opcode_wire == candidOpcode.Opt || opcode_wire == candidOpcode.Vec) {
      msg.append(
          "\n- content_opcode_wire = " + std::to_string(content_opcode_wire) +
          " (" + content_name_wire + ")");
    }
    ICPP_HOOKS::debug_print(msg);
  }

  decoder = nullptr;
  bool error{false};
  std::string error_msg;
  if ((opcode_wire != candidOpcode.Opt && opcode_wire == opcode_expected) ||
      (opcode_wire == candidOpcode.Opt && opcode_expected == candidOpcode.Opt &&
       content_opcode_wire == content_opcode_expected) ||
      (opcode_wire == candidOpcode.Vec && opcode_expected == candidOpcode.Vec &&
       content_opcode_wire == content_opcode_expected)) {
    // All good, decode it with expected args
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "--> This arg is expected and found on wire.\n    We will decode M & store the data.");
    }
    decoder = m_A.m_args_ptrs[i];
    decoder_name = "expected";
    // Fill the field _wire info used by decode_M
    if ((opcode_expected == candidOpcode.Record) ||
        (opcode_expected == candidOpcode.Variant)) {
      // Record, Variant
      int datatype_wire = m_args_datatypes_wire[j];
      auto p_wire = m_typetables_wire[datatype_wire].get_p_wire();
      decoder->set_fields_wire(p_wire);
    } else if (opcode_expected == candidOpcode.Opt &&
               content_opcode_wire == candidOpcode.Vec) {
      // TODO: add same logic here/Record/Variant for OptVecVariant
      // OptVec
      CandidType c_decoder = decoder->toCandidType();
      CandidTypeOptVec *p_opt_vec = std::get_if<CandidTypeOptVec>(&c_decoder);
      if (p_opt_vec) {
        int content_datatype_wire = m_args_content_datatypes_wire[j];
        auto p_content_wire =
            m_typetables_wire[content_datatype_wire].get_p_wire();
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
          error = true;
          error_msg = "p_content_wire is a nullptr, likely a bug.";
        }
      }
    } else if (content_opcode_wire == candidOpcode.Record) {
      if (opcode_expected == candidOpcode.Opt ||
          opcode_expected == candidOpcode.Vec) {
        // OptRecord, VecRecord
        int content_datatype_wire = m_args_content_datatypes_wire[j];
        auto p_content_wire =
            m_typetables_wire[content_datatype_wire].get_p_wire();
        if (p_content_wire) {
          CandidType c_decoder = decoder->toCandidType();
          if (opcode_expected == candidOpcode.Opt) {
            CandidTypeOptRecord *p_opt_record =
                std::get_if<CandidTypeOptRecord>(&c_decoder);
            if (p_opt_record) {
              p_opt_record->get_pv()->set_fields_wire(p_content_wire);
            } else {
              error = true;
              error_msg = "p_opt_record is a nullptr, likely a bug.";
            }
          } else if (opcode_expected == candidOpcode.Vec) {
            // A VecRecord uses a dummy record during decoding
            CandidTypeVecRecord *p_vec_record =
                std::get_if<CandidTypeVecRecord>(&c_decoder);
            if (p_vec_record) {
              p_vec_record->get_pr()->set_fields_wire(p_content_wire);
              // not used
              p_vec_record->get_pv()->set_fields_wire(p_content_wire);
            } else {
              error = true;
              error_msg = "p_vec_record is a nullptr, likely a bug.";
            }
          } else {
            error = true;
            error_msg = "ERROR: ... ";
          }
        } else {
          error = true;
          error_msg = "p_content_wire is a nullptr, likely a bug.";
        }
      } else {
        error = true;
        error_msg = "We do NOT yet handle this.";
      }
    } else if (content_opcode_wire == candidOpcode.Variant) {
      if (opcode_expected == candidOpcode.Opt ||
          opcode_expected == candidOpcode.Vec) {
        // OptVariant, VecVariant
        int content_datatype_wire = m_args_content_datatypes_wire[j];
        auto p_content_wire =
            m_typetables_wire[content_datatype_wire].get_p_wire();
        if (p_content_wire) {
          CandidType c_decoder = decoder->toCandidType();
          if (opcode_expected == candidOpcode.Opt) {
            CandidTypeOptVariant *p_opt_variant =
                std::get_if<CandidTypeOptVariant>(&c_decoder);
            if (p_opt_variant) {
              p_opt_variant->get_pv()->set_fields_wire(p_content_wire);
            } else {
              error = true;
              error_msg = "p_opt_variant is a nullptr, likely a bug.";
            }
          } else if (opcode_expected == candidOpcode.Vec) {
            ICPP_HOOKS::trap("TODO: Implement for CandidTypeVecVariant.");
            // // A VecVariant uses a dummy record during decoding
            // CandidTypeVecVariant *p_vec_variant =
            //     std::get_if<CandidTypeVecVariant>(&c_decoder);
            // if (p_vec_variant) {
            //   p_vec_variant->get_pr()->set_fields_wire(p_content_wire);
            //   // not used
            //   p_vec_variant->get_pv()->set_fields_wire(p_content_wire);
            // } else {
            //   error = true;
            //   error_msg = "p_vec_variant is a nullptr, likely a bug.";
            // }
          } else {
            error = true;
            error_msg = "ERROR: ... ";
          }
        } else {
          error = true;
          error_msg = "p_content_wire is a nullptr, likely a bug.";
        }
      } else {
        error = true;
        error_msg = "We do NOT yet handle this.";
      }
    }
  } else if (opcode_wire == candidOpcode.Null &&
             opcode_expected == candidOpcode.Opt) {
    // Special case, Expected opt found as '(null)'
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "--> This arg is an expected Opt and found as 'null' on wire.\n    There is nothing to decode.");
    }
    decoder = nullptr; // Nothing to decode
    decoder_name = "expected-opt-as-null";
  } else if (opcode_wire == candidOpcode.Opt &&
             opcode_expected == candidOpcode.Opt &&
             content_opcode_wire == candidOpcode.Null) {
    // Special case, Expected opt found as '(opt (null : null))'
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "--> This arg is an expected Opt and found as '(opt (null : null))' on wire.\n    We will decode M & discard the data.");
    }
    decoder = build_decoder_wire_for_additional_opt_arg(j);
    decoder_name = "expected-opt-as-opt-null";
  } else if (opcode_wire == candidOpcode.Opt) {
    // Additional Opt on wire must be decoded and discarded
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "--> This arg is an unexpected additional Opt found on wire.\n    We will decode M & discard the data.");
    }
    decoder = build_decoder_wire_for_additional_opt_arg(j);
    decoder_name = "read-and-discard-additional-wire-opt";
  } else if (opcode_expected == candidOpcode.Opt) {
    // Expected Opt not found on wire. Skip it and continue
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      ICPP_HOOKS::debug_print(
          "--> This arg is an expected Opt that we cannot find on the wire.\n    We will skip the expected arg.");
    }
    decoder = nullptr;
    decoder_name = "skip-expected-opt-not-found-on-wire";
  } else {
    error = true;
    error_msg = "Wrong opcode found on wire.";
  }

  if (error) {
    // Wrong type on wire
    // TODO:  IMPLEMENT CHECK ON COVARIANCE/CONTRAVARIANCE
    std::string msg;
    msg.append("ERROR: " + error_msg + "\n");
    msg.append("\nexpected arg at index " + std::to_string(i));
    msg.append("\n- Opcode   = " + std::to_string(opcode_expected) + " (" +
               candidOpcode.name_from_opcode(opcode_expected) + ")");

    msg.append("\nnext arg on wire at index " + std::to_string(j));
    msg.append("\n- datatype = " + std::to_string(datatype_wire));
    msg.append("\n- Opcode   = " + std::to_string(opcode_wire) + " (" +
               candidOpcode.name_from_opcode(opcode_wire) + ")");
    if (datatype_wire >= 0) {
      msg.append("\n- (walked the type-tables to find that Opcode!)");
    }
    ICPP_HOOKS::trap(msg);
  }
}

std::shared_ptr<CandidTypeRoot>
CandidDeserialize::build_decoder_wire_for_additional_opt_arg(int j) {
  // Build the decoder for an additional opt arg on the wire
  CandidOpcode candidOpcode;
  int datatype_wire = m_args_datatypes_wire[j];
  int opcode_wire = m_args_opcodes_wire[j];
  int content_datatype_wire = m_args_content_datatypes_wire[j];
  int content_opcode_wire = m_args_content_opcodes_wire[j];

  if (opcode_wire != candidOpcode.Opt) {
    ICPP_HOOKS::trap(
        "ERROR: this method should only be called for an additional Opt arg on the wire.");
  }

  std::shared_ptr<CandidTypeRoot> p_wire{nullptr};
  if (datatype_wire >= 0) {
    // For types with a type-table, like Record & Variant,
    // the decoder was created during deserialization of the type table section
    p_wire = m_typetables_wire[datatype_wire].get_p_wire();
  } else if (content_opcode_wire == CANDID_UNDEF) {
    ICPP_HOOKS::trap(
        "ERROR: cannot build the decoder_wire because content_opcode_wire is not set.");
  } else {
    // Primitives with simple decoder (i.e. no fields)
    CandidTypeTable *p_content_type_table = nullptr;
    if (content_datatype_wire >= 0) {
      p_content_type_table = &m_typetables_wire[content_datatype_wire];
    }
    p_wire = candidOpcode.candid_type_opt_from_opcode(content_opcode_wire,
                                                      p_content_type_table);
    p_wire->set_content_datatype(content_opcode_wire);
    p_wire->set_is_internal(true);
  }
  return p_wire;
}

// Assert candid VecBytes against a string in "hex" format (didc encode)
int CandidDeserialize::assert_candid(const std::string &candid_expected,
                                     const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}

int CandidDeserialize::get_opcode_from_datatype_on_wire(int datatype) {
  int opcode = datatype;

  if (opcode >= 0) {
    // A non-negative opcode is an index in the type tables
    // -> try at most num_typetables_wire times to get a negative opcode
    int tries = 0;
    while (opcode >= 0 && tries < m_typetables_wire.size()) {
      if (opcode < m_typetables_wire.size()) {
        opcode = m_typetables_wire[opcode].get_opcode();
      }
      tries++;
    }

    if (opcode >= 0) {
      ICPP_HOOKS::trap(
          "ERROR: Cannot find a negative Opcode by walking the type tables for type table index " +
          std::to_string(datatype));
    }
  }

  return opcode;
}