// The class for the Candid Type: vec

#include "candid_type_vec_float64.h"
#include "candid_assert.h"
#include "candid_opcode.h"

#include <cassert>



CandidTypeVecFloat64::CandidTypeVecFloat64() : CandidTypeVecBase() {
  std::vector<double> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecFloat64::CandidTypeVecFloat64(std::vector<double> *p_v)
    : CandidTypeVecBase() {
  set_pv(p_v);

  const std::vector<double> v = const_cast<std::vector<double> &>(*p_v);
  set_v(v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecFloat64::CandidTypeVecFloat64(const std::vector<double> v)
    : CandidTypeVecBase() {
  set_v(v);
  initialize();
}

CandidTypeVecFloat64::~CandidTypeVecFloat64() {}

void CandidTypeVecFloat64::set_content_type() {
  m_content_type_opcode = CandidOpcode().Float64;
  m_content_type_hex = OpcodeHex().Float64;
  m_content_type_textual = OpcodeTextual().Float64;
}

void CandidTypeVecFloat64::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  // M(z : float<N>) = f<N>(z)
  for (double const &c : m_v) {
    m_M.append_float_ieee754(c);
  }
}

// Decode the values, starting at & updating offset
bool CandidTypeVecFloat64::decode_M(VecBytes B, __uint128_t &offset,
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
  double v;
  offset_start = offset;
  parse_error = "";
  for (size_t i = 0; i < size_vec; ++i) {
    if (B.parse_float_ieee754(offset, v, parse_error)) {
      std::string to_be_parsed = "Vec: Value for CandidTypeFloat64";
      CandidAssert::trap_with_parse_error(offset_start, offset,
                                               to_be_parsed, parse_error);
    }
    m_v.push_back(v);
  }

  // Fill the user's data placeholder, if a pointer was provided
  if (m_pv) *m_pv = m_v;

  return false;
}