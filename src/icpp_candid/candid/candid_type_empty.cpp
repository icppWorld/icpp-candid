// The class for the Primitive Candid Type: empty

#include "candid_type.h"
#include "candid_type_empty.h"
#include "icpp_hooks.h"

#include "candid_opcode.h"

CandidTypeEmpty::CandidTypeEmpty() {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeEmpty::~CandidTypeEmpty() {}

void CandidTypeEmpty::set_datatype() {
  m_datatype_opcode = CandidOpcode().Empty;
  m_datatype_hex = OpcodeHex().Empty;
  m_datatype_textual = OpcodeTextual().Empty;
}

void CandidTypeEmpty::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeEmpty::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // NB: M(_ : empty) just ignore
  // ICPP_HOOKS::trap("ERROR - CandidTypeEmpty cannot have a value to encode.");
}

// Decode the values, starting at & updating offset
bool CandidTypeEmpty::decode_M(CandidDeserialize &de, VecBytes B,
                               __uint128_t &offset, std::string &parse_error) {
  ICPP_HOOKS::trap("ERROR - CandidTypeEmpty cannot have a value to decode.");
  return false;
}