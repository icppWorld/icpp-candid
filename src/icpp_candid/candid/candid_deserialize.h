// Candid deserialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results
//
// Decodes a message at the types of the CandidType objects in a CandidArgs,
// with CandidDecoder, and fills their bound C++ variables. Traps when the
// message is not valid Candid, or does not decode at those types.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "candid_args.h"

class CandidDeserialize {
public:
  CandidDeserialize();
  CandidDeserialize(const VecBytes &B, CandidArgs A);
  // Takes over the bytes, without a copy; get_B() of a temporary gives them back
  CandidDeserialize(VecBytes &&B, CandidArgs A);
  CandidDeserialize(const std::string hex_string, CandidArgs A);

  CandidArgs get_A() { return m_A; }
  std::string get_hex_string_wire() { return m_hex_string_wire; }
  // Without a copy: a reference, or the bytes moved out of a temporary
  const VecBytes &get_B() const & { return m_B; }
  VecBytes get_B() && { return std::move(m_B); }

  // Limits for a message from an untrusted caller, so that it cannot make the
  // decoder recurse or loop without end, at the canister's cost.
  // The nesting depth of values, which bounds the decoder's recursion.
  static constexpr int MAX_NESTING_DEPTH = 128;
  // The values in a message that take no bytes on the wire: null, reserved and
  // records of only those. Their count is not bounded by the message size, and
  // shared record types can make it grow exponentially with the nesting depth
  static constexpr __uint128_t MAX_ZERO_WIDTH_VALUES = 1000000;

  int assert_candid(const std::string &candid_expected,
                    const bool &assert_value);

private:
  void deserialize();

  // The vector with placeholders for the expected arguments
  CandidArgs m_A;

  // The byte stream to be deserialized
  std::string m_hex_string_wire;
  VecBytes m_B;
};
