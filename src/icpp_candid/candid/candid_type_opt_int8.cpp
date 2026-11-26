// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_int8.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptInt8::CandidTypeOptInt8() : CandidTypeBase() {
  std::optional<int8_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptInt8::CandidTypeOptInt8(std::optional<int8_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  set_v(*p_v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptInt8::CandidTypeOptInt8(std::optional<int8_t> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeOptInt8::set_content_type() {
  m_content_opcode = CandidOpcode().Int8;
  m_content_hex = OpcodeHex().Int8;
  m_content_textual = opcode_textual().Int8;
}

void CandidTypeOptInt8::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_int_fixed_width(m_v.value());
  }
}

//------------
// Initialize things
void CandidTypeOptInt8::initialize() {
  set_datatype();
  set_content_type();
  encode_M();
}

void CandidTypeOptInt8::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = opcode_textual().Opt;
}
