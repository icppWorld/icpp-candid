// The class for the Candid Type: opt
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_opt_text.h"
#include "candid_type_text.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptText::CandidTypeOptText() : CandidTypeBase() {
  std::optional<std::string> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptText::CandidTypeOptText(std::optional<std::string> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  const std::optional<std::string> v =
      const_cast<std::optional<std::string> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeOptText::CandidTypeOptText(const std::optional<std::string> v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

CandidTypeOptText::~CandidTypeOptText() {}

void CandidTypeOptText::set_content_type() {
  m_content_opcode = CandidOpcode().Text;
  m_content_hex = OpcodeHex().Text;
  m_content_textual = OpcodeTextual().Text;
}

void CandidTypeOptText::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  if (!m_v.has_value()) {
    m_M.append_int_fixed_width(uint8_t{0});
  } else {
    m_M.append_int_fixed_width(uint8_t{1});

    std::string s = m_v.value();
    // encoded size of string - leb128(|utf8(t)|)
    m_M.append_uleb128(__uint128_t(s.size()));

    // encoded string - i8*(utf8(t))
    for (char const &c : s) {
      m_M.append_byte((std::byte)c);
    }
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeOptText::decode_M(CandidDeserialize &de, VecBytes B,
                                 __uint128_t &offset,
                                 std::string &parse_error) {
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
    offset_start = offset;
    parse_error = "";
    CandidTypeText c{}; // dummy so we can use it's decode_M
    if (c.decode_M(de, B, offset, parse_error)) {
      std::string to_be_parsed = "Opt: Value for CandidTypeText";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    m_v = c.get_v();

  } else if (tag != 0) {
    ICPP_HOOKS::trap("ERROR: tag in opt text coming from wire is not 0 or 1");
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}

//------------
// Initialize things
void CandidTypeOptText::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeOptText::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// build the type table encoding
void CandidTypeOptText::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_byte((std::byte)m_content_hex);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Decode the type table, starting at & updating offset
bool CandidTypeOptText::decode_T(VecBytes B, __uint128_t &offset,
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
void CandidTypeOptText::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}
