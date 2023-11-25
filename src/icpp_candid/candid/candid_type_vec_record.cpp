// The class for the Candid Type: vec : record
#include "candid_type.h"
#include "candid_type_visitors.h"

#include "candid_serialize_type_table_registry.h"
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

void CandidTypeVecRecord::create_dummy_record_encode() {
  if (m_pr) {
    ICPP_HOOKS::trap("ERROR: This method should be called only once - " +
                     std::string(__func__));
  }
  // Use make_shared to create the dummy CandidTypeRecord and initialize the shared_ptr
  m_pr = std::make_shared<CandidTypeRecord>();

  // These fields are of type CandidTypeVecXXX
  auto field_ptrs = m_v.get_field_ptrs();
  auto field_names = m_v.get_field_names();
  auto field_ids = m_v.get_field_ids();
  CandidOpcode candidOpcode;
  // Transpose the input of Record-of-Vecs to Vec-of-Records
  // Just use the first entry in the vectors
  size_t i = 0;
  // Loop over all the vectors at index i=0 and encode a record
  for (size_t j = 0; j < field_ptrs.size(); ++j) {
    auto field_ptr = field_ptrs[j];
    auto field_name = field_names[j];
    auto field_id = field_ids[j];

    // Check that we indeed have a CandidTypeVecXXX
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

    auto content_opcode = field_ptr->get_content_opcode(); // debug only
    CandidType c_field = field_ptr->toCandidType();

    CandidType c_fields_vector_value_at_index_i =
        std::visit(GetCandidTypeVecIndexVisitor(i), c_field);

    if (field_name != "") {
      m_pr->append(field_name, c_fields_vector_value_at_index_i);
    } else {
      m_pr->append(field_id, c_fields_vector_value_at_index_i);
    }
  }

  // store it as m_pvs (root)
  m_pvs = m_pr;
}

// build the type table encoding
void CandidTypeVecRecord::encode_T() {
  m_T.append_byte((std::byte)m_datatype_hex);

  // Build a dummy Record which will also create a type-table of a dummy record in the registry
  create_dummy_record_encode();

  // Ok, so now we have a type-table in the registry for the Record
  //     that we can reference from our Vec-of-Records type table
  int c_record_type_table_index = m_pvs->get_type_table_index();
  m_T.append_sleb128(__int128_t(c_record_type_table_index));

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
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

// Decode the values, starting at & updating offset
bool CandidTypeVecRecord::decode_M(CandidDeserialize &de, VecBytes B,
                                   __uint128_t &offset,
                                   std::string &parse_error) {
  std::string debug_hex_string;
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVecRecord::decode_M");
    debug_hex_string = B.as_hex_string();
  }

  // get size of vec - leb128(N)
  __uint128_t offset_start = offset;
  __uint128_t numbytes;
  parse_error = "";
  __uint128_t size_vec_wire;
  if (B.parse_uleb128(offset, size_vec_wire, numbytes, parse_error)) {
    std::string to_be_parsed = "Size of vec- leb128(N)";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  // Loop over the vector size just read from the wire
  for (size_t i = 0; i < size_vec_wire; ++i) {
    if (CANDID_DESERIALIZE_DEBUG_PRINT) {
      std::string msg;
      msg.append(
          "Start reading data by calling record's decode_M for vec index " +
          std::to_string(i));
      ICPP_HOOKS::debug_print(msg);
    }
    offset_start = offset;
    parse_error = "";
    if (m_pvs->decode_M(de, B, offset, parse_error)) {
      std::string to_be_parsed = "VecRecord: Value for CandidTypeRecord";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }

    if (m_pv) {
      // Now map the Record fields read from the wire onto the passed in vectors
      // Note: We only support a VecRecord that can be transposed into a Record of Vecs
      for (size_t j = 0; j < m_pvs->get_field_ptrs().size(); ++j) {

        // The CandidTypeXXX just decoded from the wire
        auto &c_field_wire = *m_pvs->get_field_ptrs()[j];

        // The user-provided CandidTypeVecXXX to fill with that value
        auto &c_field_vec = *m_pv->get_field_ptrs()[j];

        // Use the push_back_value directly
        c_field_vec.push_back_value(c_field_wire);
      }
    }
  }

  // if (m_pv) {
  //   // For consistency, also copy the data stored in m_pv to m_v
  //   m_v = *m_pv;
  // }

  return false;
}

// ---------
// Initialize things
void CandidTypeVecRecord::initialize() {
  set_datatype();
  set_content_type();
  encode_T();
  encode_I();
  encode_M();
}

void CandidTypeVecRecord::set_datatype() {
  m_datatype_opcode = CandidOpcode().Vec;
  m_datatype_hex = OpcodeHex().Vec;
  m_datatype_textual = OpcodeTextual().Vec;
}

// Decode the type table, starting at & updating offset
bool CandidTypeVecRecord::decode_T(VecBytes B, __uint128_t &offset,
                                   std::string &parse_error) {
  if (CANDID_DESERIALIZE_DEBUG_PRINT) {
    ICPP_HOOKS::debug_print("+++++");
    ICPP_HOOKS::debug_print("Entered CandidTypeVecRecord::decode_T");
    ICPP_HOOKS::debug_print(
        "Start deserialization of a VecRecords' Type Table");
    ICPP_HOOKS::debug_print("offset = " + ICPP_HOOKS::to_string_128(offset));
  }

  __uint128_t len = B.size() - offset;

  // The opcode for content type, which is a type table index for a Record
  __uint128_t offset_start = offset;
  parse_error = "";
  __int128_t content_type;
  __uint128_t numbytes;
  if (B.parse_sleb128(offset, content_type, numbytes, parse_error)) {
    std::string to_be_parsed = "Type table: a Vec's content type";
    CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                        parse_error);
  }

  m_content_opcode = int(content_type);
  return false;
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
