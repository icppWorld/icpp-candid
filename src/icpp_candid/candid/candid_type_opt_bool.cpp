// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_bool.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptBool::CandidTypeOptBool() : CandidTypeBase() {
  std::optional<bool> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptBool::CandidTypeOptBool(std::optional<bool> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<bool> v = const_cast<std::optional<bool> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptBool::CandidTypeOptBool(const std::optional<bool> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptBool::~CandidTypeOptBool() {}

void CandidTypeOptBool::set_content_type() {
  m_content_opcode = CandidOpcode().Bool;
  m_content_hex = OpcodeHex().Bool;
  m_content_textual = OpcodeTextual().Bool;
}

void CandidTypeOptBool::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_uleb128(m_v.value());
  }
}

//------------
// Initialize things
void CandidTypeOptBool::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptBool::set_datatype() {
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
void CandidTypeOptBool::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
