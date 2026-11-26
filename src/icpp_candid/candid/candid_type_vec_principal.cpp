// The class for the Candid Type: vec
#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_principal.h"
#include "candid_type_vec_principal.h"

#include <cassert>

CandidTypeVecPrincipal::CandidTypeVecPrincipal() : CandidTypeBase() {
  std::vector<std::string> v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecPrincipal::CandidTypeVecPrincipal(std::vector<std::string> *p_v)
    : CandidTypeBase() {
  set_pv(p_v);

  set_v(*p_v);
  initialize();
}

// These constructors are only for encoding
CandidTypeVecPrincipal::CandidTypeVecPrincipal(std::vector<std::string> v)
    : CandidTypeBase() {
  set_v(std::move(v));
  initialize();
}

void CandidTypeVecPrincipal::set_content_type() {
  m_content_opcode = CandidOpcode().Principal;
  m_content_hex = OpcodeHex().Principal;
  m_content_textual = opcode_textual().Principal;
}

void CandidTypeVecPrincipal::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  m_M.append_uleb128(__uint128_t(m_v.size()));

  // encoded vec values - M(v : <datatype>)^N
  for (std::string const &s : m_v) {
    CandidTypePrincipal c{s};
    m_M.append(c.get_M());
  }
}

// ---------
// Initialize things
void CandidTypeVecPrincipal::initialize() {
  set_datatype();
  set_content_type();
  encode_M();
}

void CandidTypeVecPrincipal::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}
