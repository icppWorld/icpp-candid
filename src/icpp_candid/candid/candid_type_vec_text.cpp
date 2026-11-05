// The class for the Candid Type: vec
#include "candid_type.h"


#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_text.h"
#include "candid_type_vec_text.h"

#include <cassert>

CandidTypeVecText::CandidTypeVecText() : CandidTypeBase() {
  std::vector<std::string> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecText::CandidTypeVecText(std::vector<std::string> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<std::string> v =
      const_cast<std::vector<std::string> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecText::CandidTypeVecText(const std::vector<std::string> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecText::~CandidTypeVecText() {}

void CandidTypeVecText::set_content_type() {
  m_content_opcode = CandidOpcode().Text;
  m_content_hex = OpcodeHex().Text;
  m_content_textual = OpcodeTextual().Text;
}

void CandidTypeVecText::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(t : text)     = leb128(|utf8(t)|) i8*(utf8(t))
  for (std::string const &s : m_v) {
    // encoded size of string - leb128(|utf8(t)|)
    m_M.append_uleb128(__uint128_t(s.size()));

    // encoded string - i8*(utf8(t))
    for (char const &c : s) {
      m_M.append_byte((std::byte)c);
    }
  }
}

// ---------
// Initialize things
void CandidTypeVecText::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecText::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecText::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
