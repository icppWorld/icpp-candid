// The class for the Candid Type: func - a reference to a method of a canister
#include "candid_type.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_func.h"

#include <cassert>

namespace {
std::shared_ptr<CandidTypeRoot> to_root_ptr(CandidType c) {
  return std::visit(
      [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
        auto derivedPtr = std::make_shared<std::decay_t<decltype(arg_)>>(
            std::forward<decltype(arg_)>(arg_));
        return std::static_pointer_cast<CandidTypeRoot>(derivedPtr);
      },
      c);
}
} // namespace

CandidTypeFunc::CandidTypeFunc() : CandidTypeBase() {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeFunc::CandidTypeFunc(const CandidTypePrincipal &canister,
                               const std::string &method)
    : CandidTypeBase(), m_canister(canister), m_method(method) {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeFunc::~CandidTypeFunc() {}

void CandidTypeFunc::append_arg(CandidType arg_type) {
  m_arg_ptrs.push_back(to_root_ptr(arg_type));
  encode_T();
}

void CandidTypeFunc::append_ret(CandidType ret_type) {
  m_ret_ptrs.push_back(to_root_ptr(ret_type));
  encode_T();
}

void CandidTypeFunc::set_query() {
  m_annotations = {1};
  encode_T();
}

void CandidTypeFunc::set_oneway() {
  m_annotations = {2};
  encode_T();
}

void CandidTypeFunc::set_composite_query() {
  m_annotations = {3};
  encode_T();
}

void CandidTypeFunc::set_datatype() {
  m_datatype_opcode = CandidOpcode().Func;
  m_datatype_hex = OpcodeHex().Func;
  m_datatype_textual = OpcodeTextual().Func;
}

void CandidTypeFunc::append_type_reference(
    std::shared_ptr<CandidTypeRoot> p_type) {
  VecBytes T = p_type->get_T();
  if (T.size() > 0) {
    // For <comptypes>, the index into the type table
    m_T.append_sleb128(__int128_t(p_type->get_type_table_index()));
  } else {
    // For <primtypes>, the opcode
    VecBytes I = p_type->get_I();
    for (std::byte b : I.vec()) {
      m_T.append_byte(b);
    }
  }
}

// (re-)build the type table encoding
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// T(func (<datatype1>*) -> (<datatype2>*) <funcann>*) =
//   sleb128(-22) T*(<datatype1>*) T*(<datatype2>*) T*(<funcann>*)
void CandidTypeFunc::encode_T() {
  m_T.clear();
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_uleb128(__uint128_t(m_arg_ptrs.size()));
  for (auto p : m_arg_ptrs)
    append_type_reference(p);
  m_T.append_uleb128(__uint128_t(m_ret_ptrs.size()));
  for (auto p : m_ret_ptrs)
    append_type_reference(p);
  m_T.append_uleb128(__uint128_t(m_annotations.size()));
  for (uint8_t a : m_annotations)
    m_T.append_byte((std::byte)a);

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Encode the datatype
void CandidTypeFunc::encode_I() { m_I.append_byte((std::byte)m_datatype_hex); }

// (re-)build the value encoding
// M(id.m : func <functype>) = i8(1) M(id : service <actortype>) M(m : text)
// M(id : service <actortype>) = i8(1) M(id : vec nat8)
void CandidTypeFunc::encode_M() {
  m_M.clear();
  m_M.append_byte((std::byte)1);
  m_M.append_byte((std::byte)1);
  std::vector<uint8_t> id = m_canister.get_v_bytes().vec_uint8_t();
  m_M.append_uleb128(__uint128_t(id.size()));
  for (uint8_t b : id)
    m_M.append_byte((std::byte)b);
  m_M.append_uleb128(__uint128_t(m_method.size()));
  for (char c : m_method)
    m_M.append_byte((std::byte)c);
}

bool CandidTypeFunc::decode_M(CandidDeserialize &de, VecBytes B,
                              __uint128_t &offset, std::string &parse_error) {
  ICPP_HOOKS::trap("ERROR: decoding a CandidTypeFunc is not supported - " +
                   std::string(__func__));
  return true;
}
