// The class for the Primitive Candid Type: bool

#include "candid_type.h"
#include "candid_type_bool.h"
#include "candid_assert.h"

#include "candid_opcode.h"

CandidTypeBool::CandidTypeBool() { initialize(true); }

// This constructor allows for setting the value during Deserialization
CandidTypeBool::CandidTypeBool(bool *p_v) {
  set_pv(p_v);

  const bool v = const_cast<bool &>(*p_v);
  initialize(v);
}

// This constructor is only for encoding
CandidTypeBool::CandidTypeBool(const bool v) { initialize(v); }

CandidTypeBool::~CandidTypeBool() {}

// Initialize things
void CandidTypeBool::initialize(const bool &v) {
  m_v = v;
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

// pointer to data in caller, for storing decoded value
void CandidTypeBool::set_pv(bool *v) { m_pv = v; }

void CandidTypeBool::set_datatype() {
  m_datatype_opcode = CandidOpcode().Bool;
  m_datatype_hex = OpcodeHex().Bool;
  m_datatype_textual = OpcodeTextual().Bool;
}

void CandidTypeBool::encode_I() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
  // For <primtype>: the negative Opcode
  m_I.append_byte((std::byte)m_datatype_hex);
}

void CandidTypeBool::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(b : bool)     = i8(if b then 1 else 0)
  m_M.append_uleb128(m_v);
}

// Decode the values, starting at & updating offset
bool CandidTypeBool::decode_M(VecBytes B, __uint128_t &offset,
                              std::string &parse_error) {
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t iv{0};
  if (B.parse_uleb128(offset, iv, numbytes, parse_error)) {
    std::string to_be_parsed = "Value for CandidTypeBool";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }
  if (iv == 0) {
    m_v = false;
  } else {
    m_v = true;
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}