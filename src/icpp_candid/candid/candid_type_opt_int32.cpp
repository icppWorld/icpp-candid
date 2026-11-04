// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_int32.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptInt32::CandidTypeOptInt32() : CandidTypeBase() {
  std::optional<int32_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptInt32::CandidTypeOptInt32(std::optional<int32_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<int32_t> v = const_cast<std::optional<int32_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptInt32::CandidTypeOptInt32(const std::optional<int32_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptInt32::~CandidTypeOptInt32() {}

void CandidTypeOptInt32::set_content_type() {
  m_content_opcode = CandidOpcode().Int32;
  m_content_hex = OpcodeHex().Int32;
  m_content_textual = OpcodeTextual().Int32;
}

void CandidTypeOptInt32::encode_M() {
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
void CandidTypeOptInt32::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeOptInt32::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// build the type table encoding
void CandidTypeOptInt32::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptInt32::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
