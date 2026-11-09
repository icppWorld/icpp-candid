// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_int.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptInt::CandidTypeOptInt() : CandidTypeBase() {
  std::optional<__int128_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptInt::CandidTypeOptInt(std::optional<__int128_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<__int128_t> v =
      const_cast<std::optional<__int128_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptInt::CandidTypeOptInt(const std::optional<__int128_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptInt::~CandidTypeOptInt() {}

void CandidTypeOptInt::set_content_type() {
  m_content_opcode = CandidOpcode().Int;
  m_content_hex = OpcodeHex().Int;
  m_content_textual = OpcodeTextual().Int;
}

void CandidTypeOptInt::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_sleb128(m_v.value());
  }
}

//------------
// Initialize things
void CandidTypeOptInt::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptInt::set_datatype() {
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
void CandidTypeOptInt::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
