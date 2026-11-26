// The class for the Primitive Candid Type: reserved

#include "candid_type.h"
#include "candid_type_reserved.h"

#include "candid_opcode.h"

CandidTypeReserved::CandidTypeReserved() {

  set_datatype();
  encode_I();
  encode_M();
}

void CandidTypeReserved::set_datatype() {
  m_datatype_opcode = CandidOpcode().Reserved;
  m_datatype_hex = OpcodeHex().Reserved;
  m_datatype_textual = opcode_textual().Reserved;
}

void CandidTypeReserved::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
}

void CandidTypeReserved::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // To be ignored
  m_M.clear();
}
