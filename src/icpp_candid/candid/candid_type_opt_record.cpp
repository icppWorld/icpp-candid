// The class for the Candid Type: opt
#include <algorithm>
#include "candid_debug_config.h"

#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_base.h"
#include "candid_deserialize.h"
#include "candid_type_opt_record.h"

#include <cassert>

#include "icpp_hooks.h"

CandidTypeOptRecord::CandidTypeOptRecord() : CandidTypeBase() {
  CandidTypeRecord v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeOptRecord::CandidTypeOptRecord(CandidTypeRecord *p_v, bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire

  set_pv(static_cast<CandidTypeRoot *>(p_v));

  const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v);
  set_v(v);
  initialize();
}

// For internal use only, to create dummy instances for decoding during deserialization,
CandidTypeOptRecord::CandidTypeOptRecord(CandidTypeRoot *p_v_root,
                                         bool *has_value)
    : CandidTypeBase() {
  *has_value = false;
  m_p_has_value = has_value; // Set to true when found on wire
  set_pv(p_v_root);

  CandidTypeRecord *p_v_record = dynamic_cast<CandidTypeRecord *>(p_v_root);
  if (p_v_record) {
    // The cast was successful, it is indeed a Record
    const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v_record);
    set_v(v);
    initialize();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_root is not pointing to a CandidTypeRecord object - " +
        std::string(__func__));
  }
}

// These constructors are only for encoding
CandidTypeOptRecord::CandidTypeOptRecord(const CandidTypeRecord v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

// Internal: During deserialization for an additional wire argument (dummy !)
CandidTypeOptRecord::CandidTypeOptRecord(
    std::shared_ptr<CandidTypeRoot> p_v_root)
    : CandidTypeBase() {
  m_pvs = p_v_root;
  set_pv(p_v_root.get());
  CandidTypeRecord *p_v_record =
      dynamic_cast<CandidTypeRecord *>(p_v_root.get());
  if (p_v_record) {
    // A decoder only: copying the schema and encoding it cost memory per opt
    // that refers to it, and registered a type for serialization
    set_datatype();
    set_content_type();
  } else {
    // The cast failed, and p_v_record is nullptr
    ICPP_HOOKS::trap(
        "ERROR: p_v_root is not pointing to a CandidTypeRecord object - " +
        std::string(__func__));
  }
}

CandidTypeOptRecord::~CandidTypeOptRecord() {}

void CandidTypeOptRecord::set_content_type() {
  m_content_opcode = CandidOpcode().Record;
  m_content_hex = OpcodeHex().Record;
  m_content_textual = OpcodeTextual().Record;
}

void CandidTypeOptRecord::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(null : opt <datatype>) = i8(0)
  // M(?v   : opt <datatype>) = i8(1) M(v : <datatype>)

  m_M.append_int_fixed_width(uint8_t{1});
  m_M.append(m_v.get_M());
}

//------------
// Initialize things
void CandidTypeOptRecord::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeOptRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Opt;
  m_datatype_hex = OpcodeHex().Opt;
  m_datatype_textual = OpcodeTextual().Opt;
}

// build the type table encoding
void CandidTypeOptRecord::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // The type-table-index of the Record that is Optional
  __uint128_t m_content_datatype = m_v.get_type_table_index();
  m_T.append_sleb128(__int128_t(m_content_datatype));

  // Update the type table registry
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// For opts, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeOptRecord::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

int CandidTypeOptRecord::graph_type(CandidTypeGraph &g) {
  CandidTypeRoot *p = m_pv ? m_pv : (m_pvs ? m_pvs.get() : &m_v);
  return g.opt(p->graph_type(g));
}

void CandidTypeOptRecord::bind(const CandidValue &v) {
  bool has_value = !v.items.empty();
  if (m_p_has_value) *m_p_has_value = has_value;
  if (has_value && m_pv) m_pv->bind(v.items[0]);
}
