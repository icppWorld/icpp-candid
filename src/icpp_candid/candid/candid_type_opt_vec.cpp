// The class for the Candid Type: opt
#include <algorithm>
#include "candid_debug_config.h"

#include "candid_type.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_base.h"
#include "candid_deserialize.h"
#include "candid_type_opt_vec.h"
#include "candid_type_all_includes.h"
#include "candid_type_of.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptVec::CandidTypeOptVec() : CandidTypeBase() {
  CandidTypeNull v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
// Delegate constructors to root level
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecBool *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecFloat32 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecFloat64 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecInt *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecInt8 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecInt16 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecInt32 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecInt64 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecNat *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecNat8 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecNat16 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecNat32 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecNat64 *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecPrincipal *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecRecord *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}
CandidTypeOptVec::CandidTypeOptVec(CandidTypeVecText *p_v, bool *has_value)
    : CandidTypeOptVec(static_cast<CandidTypeRoot *>(p_v), has_value) {}

// These constructors are only for encoding. They keep a copy of the vec as
// its own class, which graph_type() needs: a CandidTypeRoot copy of it knows
// only its opcodes, not the type of a record's fields.
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecBool v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecBool>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecFloat32 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecFloat32>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecFloat64 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecFloat64>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecInt v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecInt>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecInt8 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecInt8>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecInt16 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecInt16>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecInt32 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecInt32>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecInt64 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecInt64>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecNat v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecNat>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecNat8 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecNat8>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecNat16 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecNat16>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecNat32 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecNat32>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecNat64 v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecNat64>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecPrincipal v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecPrincipal>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecRecord v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecRecord>(v));
}
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeVecText v)
    : CandidTypeBase() {
  by_value(std::make_shared<CandidTypeVecText>(v));
}

// Internal: During deserialization for an expected argument
CandidTypeOptVec::CandidTypeOptVec(CandidTypeRoot *p_v_root, bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire
  set_pv(p_v_root);
  set_v(*p_v_root);
  m_pvs = candid_type_of::root_ptr(p_v_root->toCandidType());
  initialize();
}

void CandidTypeOptVec::by_value(std::shared_ptr<CandidTypeRoot> p_v) {
  m_pvs = p_v;
  set_v(*p_v);
  initialize();
}

// Internal: During serialization
CandidTypeOptVec::CandidTypeOptVec(const CandidTypeRoot v) : CandidTypeBase() {
  set_v(v);
  initialize();
}

// Internal: During deserialization for an additional wire argument (dummy !)
CandidTypeOptVec::CandidTypeOptVec(std::shared_ptr<CandidTypeRoot> v)
    : CandidTypeBase() {
  m_pvs = v;
  set_pv(v.get());
  // A decoder only: copying the vec and encoding it cost memory per opt that
  // refers to it, and registered a type for serialization
  set_datatype();
  set_content_type();
}

CandidTypeOptVec::~CandidTypeOptVec() {}

void CandidTypeOptVec::set_content_type() {
  m_content_opcode = CandidOpcode().Vec;
  m_content_hex = OpcodeHex().Vec;
  m_content_textual = OpcodeTextual().Vec;
}

void CandidTypeOptVec::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  m_M.append_int_fixed_width(uint8_t{1});
  m_M.append(m_v.get_M());
  m_has_M = m_v.has_M();
}

//------------
// Initialize things
void CandidTypeOptVec::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeOptVec::set_datatype() {
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
void CandidTypeOptVec::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

int CandidTypeOptVec::graph_type(CandidTypeGraph &g) {
  // The type comes from this object's own copy, never from the caller's
  // pointer: that one may be gone by the time a message is serialized.
  CandidTypeRoot *p = m_pvs ? m_pvs.get() : &m_v;
  return g.opt(p->graph_type(g));
}

void CandidTypeOptVec::bind(const CandidValue &v) {
  bool has_value = !v.items.empty();
  if (m_p_has_value) *m_p_has_value = has_value;
  if (has_value && m_pv) m_pv->bind(v.items[0]);
}
