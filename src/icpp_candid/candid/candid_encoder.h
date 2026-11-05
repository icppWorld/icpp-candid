// The Candid encoder: serialisation of values at a type, as the spec defines
// it.
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#serialisation
//
// The type table is built once, from the argument types, in the order and
// with the sharing of the Rust implementation (candid 0.10, ser.rs
// TypeSerialize): depth first, each entry numbered before its children, so a
// recursive type refers back to itself. A node made with
// CandidTypeGraph::reserve() is a named type and gets its own entry, as a
// named type does in Rust; structurally equal nodes made with add() share
// one entry, as equal inline types do. So the bytes are those of
// `didc encode`.
//
// It never traps. It returns false with an error message for a value that
// does not fit its type, and for a type it cannot write. The value checks
// mirror the decoder's, including its limits, so what this encodes, the
// decoder decodes.

#pragma once

#include "candid_decoder.h"
#include "candid_type_graph.h"
#include "candid_value.h"

#include <cstdint>
#include <string>
#include <vector>

class CandidEncoder {
public:
  // A whole message: the header, then each value at its argument type
  static bool encode(const CandidTypeGraph &g, const std::vector<int> &args,
                     const std::vector<CandidValue> &values,
                     std::vector<uint8_t> &out, std::string &err,
                     const CandidDecodeLimits &limits = CandidDecodeLimits());

  // Only the header: the magic number, the type table and the argument types
  static bool
  encode_header(const CandidTypeGraph &g, const std::vector<int> &args,
                std::vector<uint8_t> &out, std::string &err,
                const CandidDecodeLimits &limits = CandidDecodeLimits());

  // Only the bytes of one value at type t of g, appended to out
  static bool
  encode_value(const CandidTypeGraph &g, int t, const CandidValue &v,
               std::vector<uint8_t> &out, std::string &err,
               const CandidDecodeLimits &limits = CandidDecodeLimits());
};
