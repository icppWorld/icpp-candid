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

CandidTypeVecOf::CandidTypeVecOf(CandidType element_type,
                                 std::vector<CandidType> elements)
    : CandidTypeBase() {
  m_p_element_type = root_ptr(std::move(element_type));
  for (auto &element : elements) {
    m_elements.push_back(root_ptr(std::move(element)));
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

void CandidTypeVecOf::initialize() {
  set_datatype();
  set_content_type();
  encode_M();
}

void CandidTypeVecOf::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = opcode_textual().Vec;
}

void CandidTypeVecOf::set_content_type() {
  m_content_opcode = m_p_element_type->get_datatype_opcode();
  m_content_hex = m_p_element_type->get_datatype_hex();
  m_content_textual = m_p_element_type->get_datatype_textual();
}

// M(v^N : vec <datatype>) = leb128(N) M(v : <datatype>)^N
void CandidTypeVecOf::encode_M() {
  m_M.clear();
  m_has_M = true;
  m_M.append_uleb128(__uint128_t(m_elements.size()));
  // An element that is not certainly of the element type is compared to it
  // in a type graph, which gets the element type once, when first needed
  CandidTypeGraph g;
  int t_element = 0;
  bool have_t_element = false;
  auto same_in_graph = [&](CandidTypeRoot &element) {
    if (!have_t_element) {
      t_element = m_p_element_type->graph_type(g);
      have_t_element = true;
    }
    return CandidTypeGraph::same(g, element.graph_type(g), g, t_element);
  };
  for (size_t i = 0; i < m_elements.size(); ++i) {
    if (!m_elements[i]->certainly_same_type(*m_p_element_type) &&
        !same_in_graph(*m_elements[i])) {
      ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - element " + std::to_string(i) +
                       " (" + m_elements[i]->get_datatype_textual() +
                       ") does not have the element type (" +
                       m_p_element_type->get_datatype_textual() + ")");
    }
    if (!m_elements[i]->has_M()) m_has_M = false;
    m_M.append(m_elements[i]->get_M());
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
