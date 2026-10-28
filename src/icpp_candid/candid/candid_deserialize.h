// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#pragma once

#include <string>
#include <vector>

#include "candid_args.h"
#include "candid_type_table.h"

class CandidDeserialize {
public:
  CandidDeserialize();
  CandidDeserialize(const VecBytes &B, CandidArgs A);
  CandidDeserialize(const std::string hex_string, CandidArgs A);
  ~CandidDeserialize();

  CandidArgs get_A() { return m_A; }
  std::string get_hex_string_wire() { return m_hex_string_wire; }
  VecBytes get_B() { return m_B; }
  std::vector<CandidTypeTable> &get_typetables_wire() {
    return m_typetables_wire;
  }
  // The type table at a type index from the wire. Traps when the index is not
  // in the type table section, so an index from the wire is never used unchecked
  CandidTypeTable &typetable_wire(int datatype);

  // Limits for a message from an untrusted caller, so that it cannot make the
  // decoder recurse or loop without end, at the canister's cost.
  // The nesting depth: each level of record, variant, vec or opt of those uses
  // about 3 KiB of stack, and must trap before the stack runs out. icpp-pro
  // links canisters with a 1 MiB stack; wasm-ld's default 64 KiB fits ~20.
  static constexpr int MAX_NESTING_DEPTH = 128;
  // The elements, in all the vecs of a message, that take no bytes on the wire
  // (null, reserved, a record of only those), so their count is not bounded by
  // the message size
  static constexpr __uint128_t MAX_ZERO_WIDTH_ELEMENTS = 1000000;
  void enter_nested();
  void leave_nested() { --m_nesting_depth; }
  // A vec declaring more elements than the bytes left can only hold zero width
  // elements: count them against MAX_ZERO_WIDTH_ELEMENTS
  void charge_zero_width_elements(__uint128_t n);
  // Whether a type on the wire is zero width, per type table index:
  // -1 not known yet, 0 no, 1 yes
  std::vector<int8_t> &zero_width_cache() { return m_zero_width_cache; }
  // Whether a value of this type on the wire takes zero bytes: null, reserved,
  // or a record of only those. Cached per type table index, so a deep chain of
  // shared record types is walked once
  bool is_zero_width(int datatype_wire);

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

  int get_opcode_from_datatype_on_wire(int datatype);

private:
  void deserialize();

  void select_decoder_or_trap(size_t i, size_t j,
                              std::shared_ptr<CandidTypeRoot> &decoder,
                              std::string &decoder_name);

  std::shared_ptr<CandidTypeRoot>
  build_decoder_wire_for_additional_opt_arg(int j);

  // The vector with placeholders for the expected arguments
  CandidArgs m_A;

  // The deserialized type tables found on wire
  std::vector<CandidTypeTable> m_typetables_wire;
  int m_nesting_depth{0};
  __uint128_t m_zero_width_elements{0};
  std::vector<int8_t> m_zero_width_cache;

  // The deserialized args found on wire
  std::vector<int> m_args_datatypes_wire;
  std::vector<int> m_args_opcodes_wire;
  std::vector<int> m_args_content_datatypes_wire;
  std::vector<int> m_args_content_opcodes_wire;
  std::vector<__uint128_t> m_args_datatypes_offset_start_wire;
  std::vector<__uint128_t> m_args_datatypes_offset_end_wire;

  // The byte stream to be deserialized
  std::string m_hex_string_wire;
  VecBytes m_B;
};

// Counts one level of nesting while a composite value is decoded
class CandidNestingGuard {
public:
  explicit CandidNestingGuard(CandidDeserialize &de) : m_de(de) {
    m_de.enter_nested();
  }
  ~CandidNestingGuard() { m_de.leave_nested(); }
  CandidNestingGuard(const CandidNestingGuard &) = delete;
  CandidNestingGuard &operator=(const CandidNestingGuard &) = delete;

private:
  CandidDeserialize &m_de;
};
