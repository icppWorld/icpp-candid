// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_nat64.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptNat64::CandidTypeOptNat64() : CandidTypeBase() {
  std::optional<uint64_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptNat64::CandidTypeOptNat64(std::optional<uint64_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<uint64_t> v = const_cast<std::optional<uint64_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptNat64::CandidTypeOptNat64(const std::optional<uint64_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptNat64::~CandidTypeOptNat64() {}

void CandidTypeOptNat64::set_content_type() {
  m_content_opcode = CandidOpcode().Nat64;
  m_content_hex = OpcodeHex().Nat64;
  m_content_textual = OpcodeTextual().Nat64;
}

void CandidTypeOptNat64::encode_M() {
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
void CandidTypeOptNat64::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptNat64::set_datatype() {
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
void CandidTypeOptNat64::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
