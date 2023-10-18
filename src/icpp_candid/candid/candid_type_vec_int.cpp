// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_int.h"

#include <cassert>

CandidTypeVecInt::CandidTypeVecInt() : CandidTypeBase() {
  std::vector<__int128_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecInt::CandidTypeVecInt(std::vector<__int128_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<__int128_t> v = const_cast<std::vector<__int128_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecInt::CandidTypeVecInt(const std::vector<__int128_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecInt::~CandidTypeVecInt() {}

void CandidTypeVecInt::set_content_type() {
  m_content_type_opcode = CandidOpcode().Int;
  m_content_type_hex = OpcodeHex().Int;
  m_content_type_textual = OpcodeTextual().Int;
}

void CandidTypeVecInt::push_back_value(CandidTypeRoot &value) {
  auto &derived_value = static_cast<CandidTypeInt &>(value);
  if (m_pv) {
    // We're deserializing
    m_pv->push_back(derived_value.get_v());
  } else {
    // We're serializing
    m_v.push_back(derived_value.get_v());
  }
}

void CandidTypeVecInt::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(i : int)      = sleb128(i)
  for (__int128_t const &c : m_v) {
    m_M.append_sleb128(c);
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecInt::decode_M(VecBytes B, __uint128_t &offset,
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
  __int128_t v;
  offset_start = offset;
  parse_error = "";
  for (size_t i = 0; i < size_vec; ++i) {
    if (B.parse_sleb128(offset, v, numbytes, parse_error)) {
      std::string to_be_parsed = "Vec: Value for CandidTypeInt";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_v.push_back(v);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

// ---------
// Initialize things
void CandidTypeVecInt::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecInt::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding
void CandidTypeVecInt::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_type_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecInt::decode_T(VecBytes B, __uint128_t &offset,
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

  m_content_type_opcode = int(content_type);
  return false;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecInt::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
