// A decoded Candid value, at an expected type.
//
// CandidDecoder produces a CandidValue for each argument, already coerced to
// the expected type by the spec's rules. CandidTextValue::parse produces one
// from the textual form. Two values of the same expected type compare equal
// when they are the same Candid value.
//
// The value's shape follows its expected type:
//   null, reserved     : nothing
//   bool               : b
//   nat, int           : n (arbitrary precision)
//   nat8 ... nat64     : u
//   int8 ... int64     : i
//   float32, float64   : f64 (a float32 is held exactly as a double)
//   text               : text (UTF-8)
//   principal, service : bytes (the principal id)
//   func               : bytes (the service's principal id) and text (method)
//   opt                : items holds 0 (null) or 1 element
//   vec                : items, or, for an element type of fixed width
//                        (bool, nat8 ... int64, float32/64), the elements
//                        packed little-endian in bytes, with packed = true
//   record             : ids and items, one per field of the expected type,
//                        sorted by id
//   variant            : ids[0] and items[0], the case

#pragma once

#include "candid_bigint.h"
#include "candid_type_graph.h"

#include <cstdint>
#include <string>
#include <vector>

struct CandidValue {
  CandidKind kind{CandidKind::Null};
  bool b{false};
  CandidBigInt n;
  uint64_t u{0};
  int64_t i{0};
  double f64{0.0};
  std::string text;
  std::vector<uint8_t> bytes;
  bool packed{false};
  CandidKind elem_kind{CandidKind::Null}; // for a packed vec
  std::vector<uint32_t> ids;
  std::vector<CandidValue> items;
  // A record field marked optional in the expected type, not in the message
  bool absent{false};

  // The number of elements of a vec, packed or not
  size_t vec_size() const;
  // Element i of a packed vec, unpacked
  CandidValue packed_element(size_t i) const;

  bool operator==(const CandidValue &o) const;
  bool operator!=(const CandidValue &o) const { return !(*this == o); }

  // The textual form, as in the spec's value grammar
  std::string to_text() const;
};

std::string candid_values_to_text(const std::vector<CandidValue> &vs);
