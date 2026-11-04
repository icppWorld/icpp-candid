// The class for the Candid Type: service - a reference to a canister
#include <algorithm>
#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_serialize_type_table_registry.h"

#include "candid_opcode.h"
#include "candid_principal_text.h"
#include "candid_type_service.h"

#include "icpp_hooks.h"

CandidTypeService::CandidTypeService() : CandidTypeBase() {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeService::CandidTypeService(const CandidTypePrincipal &canister)
    : CandidTypeBase(), m_canister(canister) {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeService::CandidTypeService(std::string *p_canister)
    : CandidTypeBase(), m_p_canister(p_canister) {
  set_datatype();
  encode_T();
  encode_I();
  encode_M();
}

CandidTypeService::~CandidTypeService() {}

void CandidTypeService::append_method(const std::string &name,
                                      const CandidTypeFunc &method) {
  auto it = std::lower_bound(
      m_methods.begin(), m_methods.end(), name,
      [](const auto &m, const std::string &n) { return m.first < n; });
  if (it != m_methods.end() && it->first == name) {
    ICPP_HOOKS::trap("ERROR: CandidTypeService - the method " + name +
                     " is appended twice");
  }
  m_methods.insert(it, {name, std::make_shared<CandidTypeFunc>(method)});
  encode_T();
}

void CandidTypeService::set_datatype() {
  m_datatype_opcode = CandidOpcode().Service;
  m_datatype_hex = OpcodeHex().Service;
  m_datatype_textual = OpcodeTextual().Service;
}

// (re-)build the type table encoding
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// T(service {<methtype>*}) = sleb128(-23) T*(<methtype>*)
// T(<name>:<datatype>) = leb128(|utf8(<name>)|) i8*(utf8(<name>)) I(<datatype>)
void CandidTypeService::encode_T() {
  m_T.clear();
  m_T.append_byte((std::byte)m_datatype_hex);
  m_T.append_uleb128(__uint128_t(m_methods.size()));
  for (const auto &[name, p_func] : m_methods) {
    m_T.append_uleb128(__uint128_t(name.size()));
    for (char c : name)
      m_T.append_byte((std::byte)c);
    m_T.append_sleb128(__int128_t(p_func->get_type_table_index()));
  }

  // Update the type table registry,
  m_type_table_index = CandidSerializeTypeTableRegistry::get_instance()
                           .add_or_replace_type_table(m_type_table_index, m_T);
}

// Encode the datatype
void CandidTypeService::encode_I() {
  m_I.append_byte((std::byte)m_datatype_hex);
}

// (re-)build the value encoding
// M(id : service <actortype>) = i8(1) M(id : vec nat8)
void CandidTypeService::encode_M() {
  m_M.clear();
  m_M.append_byte((std::byte)1);
  std::vector<uint8_t> id = m_canister.get_v_bytes().vec_uint8_t();
  m_M.append_uleb128(__uint128_t(id.size()));
  for (uint8_t b : id)
    m_M.append_byte((std::byte)b);
}

int CandidTypeService::graph_type(CandidTypeGraph &g) {
  std::vector<CandidGraphMethod> methods;
  for (const auto &[name, p_func] : m_methods)
    methods.push_back({name, p_func->graph_type(g)});
  return g.service(std::move(methods));
}

void CandidTypeService::bind(const CandidValue &v) {
  std::string canister = CandidPrincipalText::encode(v.bytes);
  m_canister = CandidTypePrincipal(canister);
  if (m_p_canister) *m_p_canister = canister;
}
