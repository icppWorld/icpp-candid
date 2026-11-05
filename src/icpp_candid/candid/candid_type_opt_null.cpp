// The class for the Candid Type: opt
#include "candid_type.h"


#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_null.h"
#include "candid_type_null.h"

#include <cassert>

#include "icpp_hooks.h"

// We only have a default constructor. Nothing we could possibly pass in that makes sense.
CandidTypeOptNull::CandidTypeOptNull() : CandidTypeBase() {
  std::optional<std::string> v; // Just store something...
  set_v(v);
  initialize();
}

CandidTypeOptNull::~CandidTypeOptNull() {}

void CandidTypeOptNull::set_content_type() {
  m_content_opcode = CandidOpcode().Null;
  m_content_hex = OpcodeHex().Null;
  m_content_textual = OpcodeTextual().Null;
}

void CandidTypeOptNull::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  // There is nothing to encode for a OptNull. Just tag it as 0
  m_M.append_int_fixed_width(uint8_t{0});
}

//------------
// Initialize things
void CandidTypeOptNull::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptNull::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptNull::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
