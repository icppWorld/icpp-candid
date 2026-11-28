// The class for the Primitive Candid Type: text

#include "candid_type.h"
#include "candid_type_text.h"
#include "candid_assert.h"
#include "candid_opcode.h"

#include <cassert>

CandidTypeText::CandidTypeText() { initialize(""); }

// These constructors allows for setting the value during Deserialization
CandidTypeText::CandidTypeText(std::string *p_v) {
  set_pv(p_v);

  initialize(*p_v);
}

// These constructors are only for encoding
CandidTypeText::CandidTypeText(const char *c) { initialize(std::string(c)); }
CandidTypeText::CandidTypeText(std::string v) { initialize(std::move(v)); }

// Initialize things
void CandidTypeText::initialize(std::string v) {
  m_v = std::move(v);
  set_datatype();
  // Encoded when it is read, or straight into a message (append_M_to)
  m_M_stale = true;
}

// pointer to data in caller, for storing decoded value
void CandidTypeText::set_pv(std::string *v) { m_pv = v; }

void CandidTypeText::set_datatype() {
  m_datatype_opcode = CandidOpcode().Text;
  m_datatype_hex = OpcodeHex().Text;
  m_datatype_textual = opcode_textual().Text;
}

void CandidTypeText::encode_M() {
  m_M.clear();
  m_has_M = encode_M_into(m_M);
}

bool CandidTypeText::encode_M_into(VecBytes &out) const {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(t : text)     = leb128(|utf8(t)|) i8*(utf8(t))
  out.append_uleb128(__uint128_t(m_v.size()));
  out.append_bytes(reinterpret_cast<const uint8_t *>(m_v.data()), m_v.size());
  return true;
}
