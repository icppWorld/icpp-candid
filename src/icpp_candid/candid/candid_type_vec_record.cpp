// The class for the Candid Type: vec : record
#include <algorithm>
#include "candid_type.h"
#include "candid_type_visitors.h"

#include "candid_deserialize.h"

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_record.h"
#include "candid_type_vec_record.h"

#include "icpp_hooks.h"

#include <cassert>

CandidTypeVecRecord::CandidTypeVecRecord() : CandidTypeBase() {
  CandidTypeRecord v;
  set_v(v);
  initialize();
}

// These constructors allows for setting the value during Deserialization
CandidTypeVecRecord::CandidTypeVecRecord(CandidTypeRecord *p_v)
    : CandidTypeBase() {
  set_pv(static_cast<CandidTypeRoot *>(p_v));

  const CandidTypeRecord v = const_cast<CandidTypeRecord &>(*p_v);
  set_v(v);
  initialize();
}

// For internal use only, to create dummy instances for decoding during deserialization,
CandidTypeVecRecord::CandidTypeVecRecord(CandidTypeRoot *p_v_root)
    : CandidTypeBase() {
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

// These constructors are only for encoding (v is a record of vecs)
CandidTypeVecRecord::CandidTypeVecRecord(const CandidTypeRecord v)
    : CandidTypeBase() {
  set_v(v);
  initialize();
}

// Internal: During deserialization for an additional wire argument (dummy !)
CandidTypeVecRecord::CandidTypeVecRecord(
    std::shared_ptr<CandidTypeRoot> p_wire_record)
    : CandidTypeBase() {
  m_pvs = p_wire_record;
  set_datatype();
  set_content_type();
  // No need to set anything else...
}

CandidTypeVecRecord::~CandidTypeVecRecord() {}

void CandidTypeVecRecord::set_content_type() {
  m_content_opcode = CandidOpcode().Record;
  m_content_hex = OpcodeHex().Record;
  m_content_textual = OpcodeTextual().Record;
}

void CandidTypeVecRecord::encode_M() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#memory
  // M(v^N  : vec <datatype>) = leb128(N) M(v : <datatype>)^N

  // encoded size of vec - leb128(N)
  // Check the size of the first vector of the Record of vecs that was passed in
  __uint128_t size_vec = 0;
  if (!m_v.get_field_ptrs().empty() && m_v.get_field_ptrs()[0]) {
    size_vec = __uint128_t(m_v.get_field_ptrs()[0]->get_v_size());
  }
  m_M.append_uleb128(size_vec);
  m_has_M = m_v.has_M();

  // encoded vec values - M(v : <datatype>)^N
  auto field_ptrs = m_v.get_field_ptrs();
  auto field_names = m_v.get_field_names();
  auto field_ids = m_v.get_field_ids();
  CandidOpcode candidOpcode;

  // Transpose the input of Record-of-Vecs to Vec-of-Records

  // Loop over the vector size
  for (size_t i = 0; i < size_vec; ++i) {
    CandidTypeRecord c_record;
    // Loop over all the fields and encode a record
    for (size_t j = 0; j < field_ptrs.size(); ++j) {
      auto field_ptr = field_ptrs[j];
      auto field_name = field_names[j];
      auto field_id = field_ids[j];

      // using the encoder of a dummy CandidTypeVecXXX
      auto field_opcode = field_ptr->get_datatype_opcode();

      if (field_opcode != candidOpcode.Vec) {
        std::string msg;
        msg.append(
            "ERROR: The field at index " + std::to_string(j) +
            " for the CandidTypeRecord passed into a CandidTypeVecRecord is not a Vec.");
        msg.append("\nInstead, the opcode = " + std::to_string(field_opcode) +
                   " (" + candidOpcode.name_from_opcode(field_opcode) + ")");
        ICPP_HOOKS::trap(msg);
      }

      // auto content_opcode = field_ptr->get_content_opcode();
      CandidType c_field = field_ptr->toCandidType();
      CandidType c_fields_vector_value_at_index_i =
          std::visit(GetCandidTypeVecIndexVisitor(i), c_field);

      c_record.append(field_id, c_fields_vector_value_at_index_i);
    }
    VecBytes m = c_record.get_M();
    m_M.append_bytes(m.vec_uint8_t().data(), m.size());
  }
}

// ---------
// Initialize things
void CandidTypeVecRecord::initialize() {
  set_datatype();
  set_content_type();
  encode_I();
  encode_M();
}

void CandidTypeVecRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// For vecs, we set the Opcode, but note that it is not used during serialization.
// At serialization time, we use the index in the overall type table.
//
// Encode the datatype
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// For <constype>: the negative Opcode
void CandidTypeVecRecord::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

// m_v is the record whose fields are the columns: vecs, one per field of the
// elements. The type comes from this object's own copy, never from the
// caller's pointer: that one may be gone by the time a message is serialized.
int CandidTypeVecRecord::graph_type(CandidTypeGraph &g) {
  CandidTypeRoot &r = m_v;
  const auto ids = r.get_field_ids();
  const auto ptrs = r.get_field_ptrs();
  std::vector<CandidGraphField> fields;
  for (size_t i = 0; i < ids.size(); ++i) {
    int t = ptrs[i]->graph_type(g);
    if (g.kind(t) != CandidKind::Vec) {
      ICPP_HOOKS::trap("ERROR: CandidTypeVecRecord - each field of the record "
                       "must be a vec, the column of that field");
    }
    CandidGraphField f;
    f.id = ids[i];
    f.type = g.node(t).inner;
    fields.push_back(f);
  }
  return g.vec(g.record(std::move(fields)));
}

void CandidTypeVecRecord::bind(const CandidValue &v) {
  if (!m_pv) return;
  const auto ids = m_pv->get_field_ids();
  const auto ptrs = m_pv->get_field_ptrs();
  for (size_t k = 0; k < v.vec_size(); ++k) {
    const CandidValue &element = v.at(k);
    for (size_t i = 0; i < ids.size(); ++i) {
      const CandidValue *f = element.field(ids[i]);
      if (!f)
        ICPP_HOOKS::trap("ERROR: CandidTypeVecRecord - no value for field " +
                         std::to_string(ids[i]));
      ptrs[i]->bind_push_back(*f);
    }
  }
}
