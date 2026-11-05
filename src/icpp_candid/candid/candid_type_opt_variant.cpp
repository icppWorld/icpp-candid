// The class for the Candid Type: opt
#include <algorithm>
#include "candid_debug_config.h"

#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_base.h"
#include "candid_deserialize.h"
#include "candid_type_opt_variant.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptVariant::CandidTypeOptVariant() : CandidTypeBase() {
  CandidTypeVariant v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptVariant::CandidTypeOptVariant(CandidTypeVariant *p_v,
                                           bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire
  set_pv(p_v);

  const CandidTypeVariant v = const_cast<CandidTypeVariant &>(*p_v);
  set_v(v);
  initialize();
}

// For internal use only, to create dummy instances for decoding during deserialization,
CandidTypeOptVariant::CandidTypeOptVariant(CandidTypeRoot *p_v_root,
                                           bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire
  set_pv(p_v_root);

  CandidTypeVariant *p_v_variant = dynamic_cast<CandidTypeVariant *>(p_v_root);
  if (p_v_variant) {
    // The cast was successful, it is indeed a Record
    const CandidTypeVariant v = const_cast<CandidTypeVariant &>(*p_v_variant);
    set_v(v);
    initialize();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_variant is not pointing to a CandidTypeVariant object - " +
        std::string(__func__));
  }
}

// These constructors are only for encoding
CandidTypeOptVariant::CandidTypeOptVariant(const CandidTypeVariant v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

// Internal: During deserialization for an additional wire argument (dummy !)
CandidTypeOptVariant::CandidTypeOptVariant(
    std::shared_ptr<CandidTypeRoot> p_v_root)
    : CandidTypeBase() {
  m_pvs = p_v_root;
  set_pv(p_v_root.get());
  CandidTypeVariant *p_v_variant =
      dynamic_cast<CandidTypeVariant *>(p_v_root.get());
  if (p_v_variant) {
    // A decoder only: copying the schema and encoding it cost memory per opt
    // that refers to it, and registered a type for serialization
    set_datatype();
    set_content_type();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_variant is not pointing to a CandidTypeVariant object - " +
        std::string(__func__));
  }
}

CandidTypeOptVariant::~CandidTypeOptVariant() {}

void CandidTypeOptVariant::set_content_type() {
  m_content_opcode = CandidOpcode().Variant;
  m_content_hex = OpcodeHex().Variant;
  m_content_textual = OpcodeTextual().Variant;
}

void CandidTypeOptVariant::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  m_M.append_int_fixed_width(uint8_t{1});
  m_M.append(m_v.get_M());
}

//------------
// Initialize things
void CandidTypeOptVariant::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptVariant::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptVariant::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

int CandidTypeOptVariant::graph_type(CandidTypeGraph &g) {
  // The type comes from this object's own copy, never from the caller's
  // pointer: that one may be gone by the time a message is serialized.
  CandidTypeRoot *p = m_pvs ? m_pvs.get() : &m_v;
  return g.opt(p->graph_type(g));
}

void CandidTypeOptVariant::bind(const CandidValue &v) {
  bool has_value = !v.items.empty();
  if (m_p_has_value) *m_p_has_value = has_value;
  if (has_value && m_pv) m_pv->bind(v.items[0]);
}
