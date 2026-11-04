// Textual Candid values: parsed, and annotated at expected types.
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#textual-values
//
// parse() reads an argument sequence such as
//     (record { foo = 42; bar = true }, opt "hi", vec { 1; 2 })
// and gives each value the expected type of its position, as dfinity/candid
// does for values it parses:
//   - a number takes the type it is read at (nat, int8, ...), and must fit;
//   - a float literal is a float64, or a float32 at float32;
//   - `opt v` and `null` are values of an option; nothing else is;
//   - a record drops fields the type does not have, and fills in a missing
//     field of type null, opt or reserved;
//   - a variant's case must be in the type;
//   - any value is a value of reserved;
//   - extra values are ignored, and missing trailing values of type null, opt
//     or reserved are filled in.
//
// The result is the same CandidValue the decoder produces, so the two can be
// compared. Type annotations inside a value, `(v : t)`, are not supported.

#pragma once

#include "candid_type_graph.h"
#include "candid_value.h"

#include <string>
#include <vector>

class CandidTextValue {
public:
  static bool parse(const std::string &text, const CandidTypeGraph &g,
                    const std::vector<int> &args, std::vector<CandidValue> &out,
                    std::string &err);
};
