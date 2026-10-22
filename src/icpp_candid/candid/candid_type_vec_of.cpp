// The class for the Candid Type: vec of any element type
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_serialize_type_table_registry.h"
#include "candid_deserialize.h"

#include "candid_debug_config.h"
#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_vec_of.h"

#include "icpp_hooks.h"

#include <utility>

namespace {
std::shared_ptr<CandidTypeRoot> root_ptr(CandidType c) {
  return std::visit(
      [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
        auto derivedPtr = std::make_shared<std::decay_t<decltype(arg_)>>(
            std::forward<decltype(arg_)>(arg_));
        return std::static_pointer_cast<CandidTypeRoot>(derivedPtr);
      },
      c);
}

bool same_type(const std::shared_ptr<CandidTypeRoot> &a,
               const std::shared_ptr<CandidTypeRoot> &b) {
  if (a->get_T().size() > 0 || b->get_T().size() > 0) {
    return a->get_T().size() > 0 && b->get_T().size() > 0 &&
           a->get_type_table_index() == b->get_type_table_index();
  }
  return a->get_datatype_opcode() == b->get_datatype_opcode();
}
} // namespace

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
  encode_T();
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

// T(vec <datatype>) = sleb128(-19) I(<datatype>)
void CandidTypeVecOf::encode_T() {
  m_T.clear();
  m_T.append_byte((std::byte)m_datatype_hex);
  if (m_p_element_type->get_T().size() > 0) {
    // A <comptype>: the index of its type table
    m_T.append_sleb128(__int128_t(m_p_element_type->get_type_table_index()));
  } else {
    // A <primtype>: its opcode
    VecBytes I = m_p_element_type->get_I();
    for (std::byte b : I.vec()) {
      m_T.append_byte(b);
    }
  }
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

void CandidTypeVecOf::encode_I() { m_I.append_byte((std::byte)m_datatype_hex); }

// M(v^N : vec <datatype>) = leb128(N) M(v : <datatype>)^N
void CandidTypeVecOf::encode_M() {
  m_M.clear();
  m_M.append_uleb128(__uint128_t(m_elements.size()));
  for (size_t i = 0; i < m_elements.size(); ++i) {
    if (!same_type(m_elements[i], m_p_element_type)) {
      ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - element " + std::to_string(i) +
                       " (" + m_elements[i]->get_datatype_textual() +
                       ") does not have the element type (" +
                       m_p_element_type->get_datatype_textual() + ")");
    }
    VecBytes M = m_elements[i]->get_M();
    for (std::byte b : M.vec()) {
      m_M.append_byte(b);
    }
  }
}

bool CandidTypeVecOf::bind_wire(CandidDeserialize &de, int datatype_wire) {
  if (datatype_wire < 0 ||
      datatype_wire >= int(de.get_typetables_wire().size())) {
    ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - the vec on the wire has no "
                     "type table");
  }
  m_content_datatype_wire =
      de.get_typetables_wire()[datatype_wire].get_content_datatype();
  return true;
}

bool CandidTypeVecOf::decode_M(CandidDeserialize &de, VecBytes B,
                               __uint128_t &offset, std::string &parse_error) {
  if (!m_make_element) {
    ICPP_HOOKS::trap(
        "ERROR: CandidTypeVecOf - decoding needs the make_element constructor");
  }
  if (m_content_datatype_wire == CANDID_UNDEF) {
    ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - not bound to the wire. A "
                     "CandidTypeVecOf can be an argument, a record field, a "
                     "variant case or a vec element");
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
  // Every element takes at least one byte, except an empty record, null or
  // reserved. Refuse a size that cannot fit, before allocating anything.
  if (size_vec_wire > B.size() - offset &&
      m_p_element_type->get_datatype_opcode() != CandidOpcode().Record &&
      m_p_element_type->get_datatype_opcode() != CandidOpcode().Null &&
      m_p_element_type->get_datatype_opcode() != CandidOpcode().Reserved) {
    ICPP_HOOKS::trap("ERROR: CandidTypeVecOf - the size of the vec on the "
                     "wire, " +
                     ICPP_HOOKS::to_string_128(size_vec_wire) +
                     ", is larger than the bytes left");
  }

  CandidOpcode candidOpcode;
  for (__uint128_t i = 0; i < size_vec_wire; ++i) {
    std::shared_ptr<CandidTypeRoot> p = root_ptr(m_make_element());
    int opcode_expected = p->get_datatype_opcode();

    if (m_content_datatype_wire >= 0) {
      // A <comptype> element: bind it to its type table on the wire
      CandidTypeTable &type_table =
          de.get_typetables_wire()[m_content_datatype_wire];
      int opcode_wire = type_table.get_opcode();
      if (opcode_wire != opcode_expected) {
        ICPP_HOOKS::trap(
            "ERROR: CandidTypeVecOf - Wrong type on wire for a "
            "vec element.\n- Opcode on wire = " +
            std::to_string(opcode_wire) + " (" +
            candidOpcode.name_from_opcode(opcode_wire) +
            ")\n- Opcode expected = " + std::to_string(opcode_expected) + " (" +
            candidOpcode.name_from_opcode(opcode_expected) + ")");
      }
      if (opcode_expected == candidOpcode.Record ||
          opcode_expected == candidOpcode.Variant) {
        p->set_fields_wire(type_table.get_p_wire());
      } else if (!p->bind_wire(de, m_content_datatype_wire)) {
        ICPP_HOOKS::trap(
            "ERROR: CandidTypeVecOf - an element of type " +
            p->get_datatype_textual() +
            " is not supported. Use a record, a variant or a CandidTypeVecOf "
            "as the element, e.g. wrap an opt in a record");
      }
    } else if (m_content_datatype_wire != opcode_expected) {
      ICPP_HOOKS::trap(
          "ERROR: CandidTypeVecOf - Wrong type on wire for a vec element.\n- "
          "Opcode on wire = " +
          std::to_string(m_content_datatype_wire) + " (" +
          candidOpcode.name_from_opcode(m_content_datatype_wire) +
          ")\n- Opcode expected = " + std::to_string(opcode_expected) + " (" +
          candidOpcode.name_from_opcode(opcode_expected) + ")");
    }

    offset_start = offset;
    parse_error = "";
    if (p->decode_M(de, B, offset, parse_error)) {
      std::string to_be_parsed = "VecOf: Value for a vec element";
      CandidAssert::trap_with_parse_error(offset_start, offset, to_be_parsed,
                                          parse_error);
    }
    if (m_element_done) m_element_done();
  }
  return false;
}
