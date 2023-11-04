// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

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

void CandidTypeVecText::push_back_value(CandidTypeRoot &value) {
  auto &derived_value = static_cast<CandidTypeText &>(value);
  if (m_pv) {
    // We're deserializing
    m_pv->push_back(derived_value.get_v());
  } else {
    // We're serializing
    m_v.push_back(derived_value.get_v());
  }
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

// Decode the values, starting at & updating offset
bool CandidTypeVecText::decode_M(CandidDeserialize &de, VecBytes B,
                                 __uint128_t &offset,
                                 std::string &parse_error) {
  // get size of vec - leb128(N)
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t size_vec;
  if (B.parse_uleb128(offset, size_vec, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of vec- leb128(N)";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  m_v.clear();
  offset_start = offset;
  parse_error = "";
  CandidTypeText c{""}; // dummy so we can use it's decode_M
  for (size_t i = 0; i < size_vec; ++i) {
    if (c.decode_M(de, B, offset, parse_error)) {
      std::string to_be_parsed = "Vec: Value for CandidTypeText";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_v.push_back(c.get_v());
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

// ---------
// Initialize things
void CandidTypeVecText::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecText::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding
void CandidTypeVecText::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecText::decode_T(VecBytes B, __uint128_t &offset,
                                 std::string &parse_error) {
  __uint128_t len = B.size() - offset;

  // The opcode for content type
  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type;
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: a Vec's content type";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  m_content_opcode = int(content_type);
  return false;
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
