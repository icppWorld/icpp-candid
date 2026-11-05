// The class for the Candid Type: vec of any element type
#include <algorithm>
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_deserialize.h"

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_of.h"
#include "candid_type_vec_of.h"

#include "icpp_hooks.h"

using candid_type_of::root_ptr;
using candid_type_of::same_type;

CandidTypeVecOf::CandidTypeVecOf(CandidType element_type,
                                 std::vector<CandidType> elements)
    : CandidTypeBase() {
  m_p_element_type = root_ptr(element_type);
  for (auto &element : elements) {
    m_elements.push_back(root_ptr(element));
  }
  m_v_size = m_elements.size();
  initialize();
}

CandidTypeVecOf::CandidTypeVecOf(std::function<CandidType()> make_element,
                                 std::function<void()> element_done)
    : CandidTypeBase() {
  m_make_element = make_element;
  m_element_done = element_done;
  m_p_element_type = root_ptr(m_make_element());
  initialize();
}

CandidTypeVecOf::~CandidTypeVecOf() {}

void CandidTypeVecOf::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecOf::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

void CandidTypeVecOf::set_content_type() {
  m_content_opcode = m_p_element_type->get_datatype_opcode();
  m_content_hex = m_p_element_type->get_datatype_hex();
  m_content_textual = m_p_element_type->get_datatype_textual();
}

void CandidTypeVecOf::encode_I() { m_I.append_byte((std::byte)m_datatype_hex); }

// M(v^N : vec <datatype>) = leb128(N) M(v : <datatype>)^N
void CandidTypeVecOf::encode_M() {
  m_M.clear();
  m_has_M = true;
  m_M.append_uleb128(__uint128_t(m_elements.size()));
  for (size_t i = 0; i < m_elements.size(); ++i) {
    if (!same_type(m_elements[i], m_p_element_type)) {
      ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - element " + std::to_string(i) +
                       " (" + m_elements[i]->get_datatype_textual() +
                       ") does not have the element type (" +
                       m_p_element_type->get_datatype_textual() + ")");
    }
    if (!m_elements[i]->has_M()) m_has_M = false;
    VecBytes M = m_elements[i]->get_M();
    for (std::byte b : M.vec()) {
      m_M.append_byte(b);
    }
  }
}

int CandidTypeVecOf::graph_type(CandidTypeGraph &g) {
  return g.vec(m_p_element_type->graph_type(g));
}

void CandidTypeVecOf::bind(const CandidValue &v) {
  if (!m_make_element) {
    ICPP_HOOKS::trap(
        "ERROR: CandidTypeVecOf - decoding needs the make_element constructor");
  }
  size_t n = v.vec_size();
  for (size_t i = 0; i < n; ++i) {
    std::shared_ptr<CandidTypeRoot> p = root_ptr(m_make_element());
    if (v.packed) p->bind(v.packed_element(i));
    else p->bind(v.at(i));
    if (m_element_done) m_element_done();
  }
}
