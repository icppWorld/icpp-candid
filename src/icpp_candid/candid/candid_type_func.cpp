// The class for the Candid Type: func - a reference to a method of a canister
#include <algorithm>
#include "candid_type.h"
#include "candid_type_all_includes.h"


#include "candid_assert.h"
#include "candid_opcode.h"
#include "candid_type_func.h"
#include "candid_type_of.h"

#include <cassert>

using candid_type_of::root_ptr;

CandidTypeFunc::CandidTypeFunc() : CandidTypeBase() {
  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeFunc::CandidTypeFunc(const CandidTypePrincipal &canister,
                               const std::string &method)
    : CandidTypeBase(), m_canister(canister), m_method(method) {
  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeFunc::CandidTypeFunc(std::string *p_canister, std::string *p_method)
    : CandidTypeBase(), m_p_canister(p_canister), m_p_method(p_method) {
  set_datatype();
  encode_I();
  encode_M();
}

CandidTypeFunc::~CandidTypeFunc() {}

void CandidTypeFunc::append_arg(CandidType arg_type) {
  m_arg_ptrs.push_back(root_ptr(arg_type));
}

void CandidTypeFunc::append_ret(CandidType ret_type) {
  m_ret_ptrs.push_back(root_ptr(ret_type));
}

void CandidTypeFunc::set_query() {
  m_annotations = {1};
}

void CandidTypeFunc::set_oneway() {
  m_annotations = {2};
}

void CandidTypeFunc::set_composite_query() {
  m_annotations = {3};
}

void CandidTypeFunc::set_datatype() {
  m_datatype_opcode = CandidOpcode().Func;
  m_datatype_hex = OpcodeHex().Func;
  m_datatype_textual = OpcodeTextual().Func;
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

int CandidTypeFunc::graph_type(CandidTypeGraph &g) {
  std::vector<int> args, rets;
  for (auto &p : m_arg_ptrs)
    args.push_back(p->graph_type(g));
  for (auto &p : m_ret_ptrs)
    rets.push_back(p->graph_type(g));
  return g.func(std::move(args), std::move(rets), m_annotations);
}

void CandidTypeFunc::bind(const CandidValue &v) {
  std::string canister = CandidPrincipalText::encode(v.bytes);
  m_canister = CandidTypePrincipal(canister);
  m_method = v.text;
  if (m_p_canister) *m_p_canister = canister;
  if (m_p_method) *m_p_method = m_method;
}
