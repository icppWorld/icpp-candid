// The class for the Primitive Candid Type: null

#include "candid_type.h"
#include "candid_type_null.h"

#include "candid_opcode.h"

CandidTypeNull::CandidTypeNull() {
  set_datatype();
  encode_I();
  encode_M();
}

void CandidTypeNull::set_datatype() {
  m_datatype_opcode = CandidOpcode().Null;
  m_datatype_hex = OpcodeHex().Null;
  m_datatype_textual = opcode_textual().Null;
}

void CandidTypeNull::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
}

void CandidTypeNull::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(_ : null)     = .

  // A null type contains no data.
  m_M.clear();
}