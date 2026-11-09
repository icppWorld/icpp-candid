// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_float64.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptFloat64::CandidTypeOptFloat64() : CandidTypeBase() {
  std::optional<double> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptFloat64::CandidTypeOptFloat64(std::optional<double> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<double> v = const_cast<std::optional<double> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptFloat64::CandidTypeOptFloat64(const std::optional<double> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptFloat64::~CandidTypeOptFloat64() {}

void CandidTypeOptFloat64::set_content_type() {
  m_content_opcode = CandidOpcode().Float64;
  m_content_hex = OpcodeHex().Float64;
  m_content_textual = OpcodeTextual().Float64;
}

void CandidTypeOptFloat64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_float_ieee754(m_v.value());
  }
}

//------------
// Initialize things
void CandidTypeOptFloat64::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptFloat64::set_datatype() {
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
void CandidTypeOptFloat64::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
