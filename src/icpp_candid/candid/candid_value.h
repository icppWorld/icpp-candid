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
//   record             : one field per field of the expected type, read with
//                        field(id) or for_each_field. The fields are in ids
//                        and items, sorted by id, and in rest (see there).
//   variant            : ids[0] and items[0], the case

#pragma once

#include "candid_bigint.h"
#include "candid_type_graph.h"

#include <cstdint>
#include <memory>
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
  // A vec whose elements take no bytes on the wire (null, reserved, records
  // of those) has elements that are all equal: it holds one, in items[0],
  // and its length here. So a message of a few bytes cannot make the decoder
  // store a million values.
  size_t repeat{0};
  // A record field marked optional in the expected type, not in the message
  bool absent{false};
  // A record's fields that take no bytes on the wire, and the expected ones
  // that are not on the wire, also sorted by id. They are the same in every
  // value of that wire type at that expected type, so the decoder makes them
  // once and shares them: a record tree of such fields, of a few bytes, holds
  // one value per level, not one per node.
  std::shared_ptr<const CandidValue> rest;

  // The number of elements of a vec, packed or not
  size_t vec_size() const;
  // Element i of a packed vec, unpacked
  CandidValue packed_element(size_t i) const;
  // Element i of a vec that is not packed
  const CandidValue &at(size_t i) const { return repeat ? items[0] : items[i]; }

  // A record's field, or nullptr
  const CandidValue *field(uint32_t id) const;
  // The number of fields of a record
  size_t field_count() const;
  // f(id, value) for each field of a record, in order of id
  template <class F> void for_each_field(F f) const;

  bool operator==(const CandidValue &o) const;
  bool operator!=(const CandidValue &o) const { return !(*this == o); }

  // The textual form, as in the spec's value grammar
  std::string to_text() const;
};

template <class F> void CandidValue::for_each_field(F f) const {
  static const std::vector<uint32_t> no_ids;
  static const std::vector<CandidValue> no_items;
  const auto &rids = rest ? rest->ids : no_ids;
  const auto &ritems = rest ? rest->items : no_items;
  size_t a = 0, r = 0;
  while (a < ids.size() || r < rids.size()) {
    if (r == rids.size() || (a < ids.size() && ids[a] < rids[r])) {
      f(ids[a], items[a]);
      ++a;
    } else {
      f(rids[r], ritems[r]);
      ++r;
    }
  }
}

std::string candid_values_to_text(const std::vector<CandidValue> &vs);
