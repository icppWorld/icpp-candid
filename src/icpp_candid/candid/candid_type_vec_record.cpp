// The class for the Candid Type: vec : record

#include "candid_type_vec_record.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_record.h"

#include "icpp_hooks.h"

#include <cassert>

CandidTypeVecRecord::CandidTypeVecRecord() : CandidTypeVecBase() {
  CandidTypeRecord v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecRecord::CandidTypeVecRecord(CandidTypeRecord *p_v)
    : CandidTypeVecBase() {
  set_pv(p_v);

  const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecRecord::CandidTypeVecRecord(const CandidTypeRecord v)
    : CandidTypeVecBase() {
  set_v(v);
  initialize();
}

CandidTypeVecRecord::~CandidTypeVecRecord() {}

void CandidTypeVecRecord::set_content_type() {
  m_content_type_opcode = CandidOpcode().Record;
  m_content_type_hex = OpcodeHex().Record;
  m_content_type_textual = OpcodeTextual().Record;
}

void CandidTypeVecRecord::encode_M() {
  ICPP_HOOKS::trap("CandidTypeVecRecord:encode_M to be implemented");
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N
  // if (m_v.m_args_ptrs.size() == 0) {
  //   ICPP_HOOKS::trap("ERROR: " + std::string(__func__) +
  //                    " - CandidTypeRecord to encode is empty.");
  // }

  // // encoded size of vec - leb128(N)
  // int opcode = m_v.m_args_ptrs[0]->get_content_type_opcode();
  // if (opcode != CandidOpcode().Vec) {
  //   ICPP_HOOKS::trap("ERROR: " + std::string(__func__) +
  //                    " - CandidTypeRecord[0] is not a CandidTypeVecXXX.");
  // }

  // auto vec =
  //     std::visit([](auto &&c) { return c.get_v(); }, *(m_v.m_args_ptrs[0]));
  // m_M.append_uleb128(__uint128_t(vec.size()));

  // encoded vec values - M(v : <datatype>)^N
  /*
  Pseudo code:
  m_v
  */
  // // M(t : text)     = leb128(|utf8(t)|) i8*(utf8(t))
  // for (std::string const &s : m_v) {
  //   // encoded size of string - leb128(|utf8(t)|)
  //   m_M.append_uleb128(__uint128_t(s.size()));

  //   // encoded string - i8*(utf8(t))
  //   for (char const &c : s) {
  //     m_M.append_byte((std::byte)c);
  //   }
  // }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecRecord::decode_M(VecBytes B, __uint128_t &offset,
                                   std::string &parse_error) {
  ICPP_HOOKS::trap("CandidTypeVecRecord:decode_M to be implemented");
  // // get size of vec - leb128(N)
  // __uint128_t offset_start = offset;
  // __uint128_t numbytes;
  // parse_error = "";
  // __uint128_t size_vec;
  // if (B.parse_uleb128(offset, size_vec, numbytes, parse_error)) {
  //   std::string to_be_parsed = "Size of vec- leb128(N)";
  //   CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
  //                                       parse_error);
  // }

  // m_v.clear();
  // offset_start = offset;
  // parse_error = "";
  // CandidTypeText c{""}; // dummy so we can use it's decode_M
  // for (size_t i = 0; i < size_vec; ++i) {
  //   if (c.decode_M(B, offset, parse_error)) {
  //     std::string to_be_parsed = "Vec: Value for CandidTypeText";
  //     CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
  //                                         parse_error);
  //   }
  //   m_v.push_back(c.get_v());
  // }

  // // Fill the user's data placeholder, if a pointer was provided
  // if (m_pv) *m_pv = m_v;

  return false;
}