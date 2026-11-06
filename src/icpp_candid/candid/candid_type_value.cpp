// The class for a Candid value of any type, recursive types included

#include "candid_type_value.h"
#include "candid_type_all_includes.h"

#include "candid_encoder.h"
#include "candid_opcode.h"

#include "icpp_hooks.h"

CandidTypeValue::CandidTypeValue(const CandidTypeGraph &g, int t,
                                 const CandidValue &v)
    : CandidTypeBase(), m_v(v) {
  initialize(g, t);
  if (!m_has_M)
    ICPP_HOOKS::trap("ERROR: CandidTypeValue - the value does not fit the "
                     "type " +
                     g.to_string(t) + ": " + m_err);
}

CandidTypeValue::CandidTypeValue(const CandidTypeGraph &g, int t,
                                 CandidValue *p_v)
    : CandidTypeBase(), m_v(*p_v), m_p_v(p_v) {
  // Its value is encoded if it fits, so it can also be sent
  initialize(g, t);
}

CandidTypeValue::CandidTypeValue(const CandidTypeGraph &g, int t)
    : CandidTypeBase() {
  initialize(g, t);
  m_has_M = false;
}

CandidTypeValue::~CandidTypeValue() {}

void CandidTypeValue::initialize(const CandidTypeGraph &g, int t) {
  std::string err;
  if (!g.validate(err))
    ICPP_HOOKS::trap("ERROR: CandidTypeValue - the type graph is not valid: " +
                     err);
  if (!g.is_valid_ref(t))
    ICPP_HOOKS::trap("ERROR: CandidTypeValue - the type is not in the graph");
  m_g = std::make_shared<const CandidTypeGraph>(g);
  m_t = t;
  set_datatype();
  encode_I();
  encode_M();
}

void CandidTypeValue::set_datatype() {
  CandidOpcode op;
  switch (m_g->kind(m_t)) {
  case CandidKind::Opt:
    m_datatype_opcode = op.Opt;
    break;
  case CandidKind::Vec:
    m_datatype_opcode = op.Vec;
    break;
  case CandidKind::Record:
    m_datatype_opcode = op.Record;
    break;
  case CandidKind::Variant:
    m_datatype_opcode = op.Variant;
    break;
  case CandidKind::Func:
    m_datatype_opcode = op.Func;
    break;
  case CandidKind::Service:
    m_datatype_opcode = op.Service;
    break;
  default:
    m_datatype_opcode = m_t; // a primitive: its opcode
    break;
  }
  m_datatype_hex = uint8_t(m_datatype_opcode & 0x7f);
  m_datatype_textual = op.name_from_opcode(m_datatype_opcode);
}

void CandidTypeValue::encode_I() { m_I.append_byte((std::byte)m_datatype_hex); }

// The value's bytes, if it fits the type. If not, m_has_M is false: such a
// value can be decoded into, not sent.
void CandidTypeValue::encode_M() {
  m_M.clear();
  std::vector<uint8_t> out;
  m_err.clear();
  m_has_M = CandidEncoder::encode_value(*m_g, m_t, m_v, out, m_err);
  if (m_has_M) m_M.append_bytes(out.data(), uint32_t(out.size()));
}

int CandidTypeValue::graph_type(CandidTypeGraph &g) {
  return g.import(*m_g, m_t);
}

// A decoded value is not encoded again: a value the decoder holds compactly
// (a vec of one repeated element) can take many more bytes than the message
// it came from. To send it, build a new CandidTypeValue with it.
void CandidTypeValue::bind(const CandidValue &v) {
  m_v = v;
  if (m_p_v) *m_p_v = v;
  m_M.clear();
  m_has_M = false;
}
