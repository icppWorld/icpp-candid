// The class for the Candid Type: opt of any type
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_opcode.h"
#include "candid_type_of.h"
#include "candid_type_opt_of.h"

#include "icpp_hooks.h"

using candid_type_of::root_ptr;
using candid_type_of::same_type;

CandidTypeOptOf::CandidTypeOptOf(CandidType content_type) : CandidTypeBase() {
  m_p_content_type = root_ptr(content_type);
  initialize();
}

CandidTypeOptOf::CandidTypeOptOf(CandidType content_type, CandidType value)
    : CandidTypeBase() {
  m_p_content_type = root_ptr(content_type);
  m_p_value = root_ptr(value);
  if (!same_type(m_p_value, m_p_content_type)) {
    ICPP_HOOKS::trap("ERROR: CandidTypeOptOf - the value (" +
                     m_p_value->get_datatype_textual() +
                     ") does not have the content type (" +
                     m_p_content_type->get_datatype_textual() + ")");
  }
  initialize();
}

CandidTypeOptOf::CandidTypeOptOf(std::function<CandidType()> make_content,
                                 bool *has_value)
    : CandidTypeBase() {
  m_make_content = make_content;
  m_p_has_value = has_value;
  m_p_content_type = root_ptr(m_make_content());
  initialize();
}

CandidTypeOptOf::~CandidTypeOptOf() {}

void CandidTypeOptOf::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptOf::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

void CandidTypeOptOf::set_content_type() {
  m_content_opcode = m_p_content_type->get_datatype_opcode();
  m_content_hex = m_p_content_type->get_datatype_hex();
  m_content_textual = m_p_content_type->get_datatype_textual();
}

void CandidTypeOptOf::encode_I() { m_I.append_byte((std::byte)m_datatype_hex); }

// M(null : opt <datatype>) = i8(0)
// M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)
void CandidTypeOptOf::encode_M() {
  m_M.clear();
  m_has_M = true;
  if (!m_p_value) {
    m_M.append_int_fixed_width(uint8_t{0});
    return;
  }
  m_M.append_int_fixed_width(uint8_t{1});
  m_M.append(m_p_value->get_M());
  m_has_M = m_p_value->has_M();
}

int CandidTypeOptOf::graph_type(CandidTypeGraph &g) {
  return g.opt(m_p_content_type->graph_type(g));
}

void CandidTypeOptOf::bind(const CandidValue &v) {
  bool has_value = !v.items.empty();
  if (m_p_has_value) *m_p_has_value = has_value;
  if (!has_value) return;
  if (!m_make_content) {
    ICPP_HOOKS::trap(
        "ERROR: CandidTypeOptOf - decoding needs the make_content constructor");
  }
  root_ptr(m_make_content())->bind(v.items[0]);
}
