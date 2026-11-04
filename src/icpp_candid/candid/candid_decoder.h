// The Candid decoder: deserialisation at an expected type, as the spec defines
// it.
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#deserialisation
//
// It checks the magic number, reads the type table into a CandidTypeGraph,
// decodes each value at its wire type and coerces it to the expected type,
// following the spec's coercion rules, including subtyping, the opt rules and
// the subtype check on func and service references.
//
// It never traps. It returns false with an error message for a message that
// is not valid Candid, or that does not coerce to the expected types. The
// callers decide what to do with that: the typed API traps, the compliance
// runner records the result.
//
// Every read is bounded by the bytes left. The work is bounded by the size of
// the message, plus a budget for values that take no bytes at all (vec null,
// records of nulls), which is what defeats a "space bomb".

#pragma once

#include "candid_type_graph.h"
#include "candid_value.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct CandidDecodeLimits {
  // Values that take no bytes on the wire (null, reserved, records of them),
  // over the whole message. Each costs time but no bytes, so without a limit
  // a few bytes could ask for billions of them.
  size_t max_zero_width_values{1000000};
  // How deeply values and types may nest
  size_t max_depth{128};
  // Entries in the type table
  size_t max_type_table{10000};
};

class CandidDecoder {
public:
  // Decode a message at the expected argument types of `expected`.
  static bool decode(const uint8_t *data, size_t size,
                     const CandidTypeGraph &expected,
                     const std::vector<int> &args,
                     std::vector<CandidValue> &out, std::string &err,
                     const CandidDecodeLimits &limits = CandidDecodeLimits());

  // True if the bytes are valid UTF-8 (no overlong forms, no surrogates, at
  // most U+10FFFF)
  static bool is_valid_utf8(const uint8_t *s, size_t n);
};
