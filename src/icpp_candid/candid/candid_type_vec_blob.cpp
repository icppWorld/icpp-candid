// The class for the Candid Type: vec blob (vec vec nat8)
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_blob.h"

#include <cassert>

CandidTypeVecBlob::CandidTypeVecBlob() : CandidTypeBase() {
  std::vector<std::vector<uint8_t>> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecBlob::CandidTypeVecBlob(std::vector<std::vector<uint8_t>> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::vector<std::vector<uint8_t>> v =
      const_cast<std::vector<std::vector<uint8_t>> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecBlob::CandidTypeVecBlob(const std::vector<std::vector<uint8_t>> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeVecBlob::~CandidTypeVecBlob() {}

void CandidTypeVecBlob::set_content_type() {
  m_content_opcode = CandidOpcode().Vec;
  m_content_hex = OpcodeHex().Vec;
  m_content_textual = OpcodeTextual().Vec;
}

// A blob, e.g. while decoding a vec record whose field is a blob
void CandidTypeVecBlob::push_back_value(CandidTypeRoot &value) {
  auto &derived_value = static_cast<CandidTypeVecNat8 &>(value);
  if (m_pv) {
    // We're deserializing
    m_pv->push_back(derived_value.get_v());
  } else {
    // We're serializing
    m_v.push_back(derived_value.get_v());
  }
}

void CandidTypeVecBlob::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N
  // with <datatype> = vec nat8: leb128(size) followed by the bytes
  m_M.append_uleb128(__uint128_t(m_v.size()));
  for (const std::vector<uint8_t> &blob : m_v) {
    m_M.append_uleb128(__uint128_t(blob.size()));
    for (uint8_t const &c : blob) {
      m_M.append_int_fixed_width(c);
    }
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecBlob::decode_M(CandidDeserialize &de, VecBytes &B,
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
  for (size_t i = 0; i < size_vec; ++i) {
    // get size of the blob - leb128(n)
    offset_start = offset;
    parse_error = "";
    __uint128_t size_blob;
    if (B.parse_uleb128(offset, size_blob, numbytes, parse_error)) {
      std::string to_be_parsed = "Size of blob in vec blob - leb128(n)";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    // parse_bytes checks that the bytes are there before it copies them, so a
    // huge size on the wire allocates nothing
    std::vector<uint8_t> blob;
    offset_start = offset;
    parse_error = "";
    if (B.parse_bytes(offset, blob, size_blob, numbytes, parse_error)) {
      std::string to_be_parsed = "VecBlob: the bytes of a blob";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_v.push_back(blob);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

// ---------
// Initialize things
void CandidTypeVecBlob::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecBlob::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// build the type table encoding: vec <index of the type table of vec nat8>
void CandidTypeVecBlob::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // The type table of the content, vec nat8, registered in the registry
  if (!m_content) m_content = std::make_shared<CandidTypeVecNat8>();
  m_T.append_sleb128(__int128_t(m_content->get_type_table_index()));

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecBlob::decode_T(VecBytes &B, __uint128_t &offset,
                                 std::string &parse_error) {
  // The content type: an index into the type table, for the vec nat8
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

// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecBlob::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
