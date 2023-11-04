// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_nat.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptNat::CandidTypeOptNat() : CandidTypeBase() {
  std::optional<__uint128_t> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptNat::CandidTypeOptNat(std::optional<__uint128_t> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<__uint128_t> v =
      const_cast<std::optional<__uint128_t> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptNat::CandidTypeOptNat(const std::optional<__uint128_t> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptNat::~CandidTypeOptNat() {}

void CandidTypeOptNat::set_content_type() {
  m_content_opcode = CandidOpcode().Nat;
  m_content_hex = OpcodeHex().Nat;
  m_content_textual = OpcodeTextual().Nat;
}

void CandidTypeOptNat::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});
    m_M.append_uleb128(m_v.value());
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeOptNat::decode_M(CandidDeserialize &de, VecBytes B,
                                __uint128_t &offset, std::string &parse_error) {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  // get tag of opt - i8(0)
  // this will also work if there is a null on the wire.
  __uint128_t offset_start = offset;
  parse_error = "";
  uint8_t tag;
  if (B.parse_int_fixed_width(offset, tag, parse_error)) {
    std::string to_be_parsed = "Opt tag.";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }
  if (tag == 1) {
    __uint128_t v;
    offset_start = offset;
    __uint128_t numbytes;
    parse_error = "";
    if (B.parse_uleb128(offset, v, numbytes, parse_error)) {
      std::string to_be_parsed = "Opt: Value for CandidTypeNat";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_v = v;

  } else if (tag != 0) {
    ICPP_HOOKS::trap("ERROR: tag in opt nat coming from wire is not 0 or 1");
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

//------------
// Initialize things
void CandidTypeOptNat::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeOptNat::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// build the type table encoding
void CandidTypeOptNat::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Decode the type table, starting at & updating offset
bool CandidTypeOptNat::decode_T(VecBytes B, __uint128_t &offset,
                                std::string &parse_error) {
  __uint128_t len = B.size() - offset;

  // The opcode for content type
  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type;
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: a Opt's content type";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  m_content_opcode = int(content_type);
  return false;
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptNat::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
