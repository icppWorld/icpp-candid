// The base class for all Candid Types

#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_opcode.h"
#include "candid_type_base.h"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <system_error>
#include <typeinfo>

#include "icpp_hooks.h"

CandidTypeRoot::CandidTypeRoot() {}

void CandidTypeRoot::insert_field(uint32_t field_id, std::string field_name,
                                  std::shared_ptr<CandidTypeRoot> p,
                                  const char *kind) {
  auto iter =
      std::lower_bound(m_field_ids.begin(), m_field_ids.end(), field_id);
  const auto i = std::distance(m_field_ids.begin(), iter);
  if (iter != m_field_ids.end() && *iter == field_id) {
    std::string msg;
    msg.append(std::string("ERROR: ") + kind +
               " already has a field with the same field_id.\n");
    msg.append("       field id (hash): " + std::to_string(field_id) + "\n");
    msg.append("       field name 1   : " + m_field_names[i] + "\n");
    msg.append("       field name 2   : " + field_name + "\n");
    ICPP_HOOKS::trap(msg);
  }
  // During encoding, the datatype is always a negative opcode !
  assert(p->get_datatype_opcode() < 0);
  m_field_ids.insert(iter, field_id);
  m_field_names.insert(m_field_names.begin() + i, std::move(field_name));
  m_field_ptrs.insert(m_field_ptrs.begin() + i, std::move(p));
}

bool CandidTypeRoot::certainly_same_type(const CandidTypeRoot &other) const {
  // A class that does not override graph_type has its primitive opcode as type
  CandidKind k;
  return m_datatype_opcode == other.m_datatype_opcode &&
         CandidTypeGraph::prim_kind(m_datatype_opcode, k) &&
         typeid(*this) == typeid(other);
}

// Virtual method to be implemented by all derived classes
CandidType CandidTypeRoot::toCandidType() {
  ICPP_HOOKS::trap("ERROR: toCandidType not implemented for " +
                   m_datatype_textual);
  return CandidTypeBool{false}; // Need to return something
}

void CandidTypeRoot::set_content_type() {
  ICPP_HOOKS::trap("ERROR: set_content_type not implemented for " +
                   m_datatype_textual);
}

// Virtual methods to be implemented by VecRecord
void CandidTypeRoot::create_dummy_record_encode() {
  ICPP_HOOKS::trap("ERROR: create_dummy_record_encode not implemented for " +
                   m_datatype_textual);
}

int CandidTypeRoot::graph_type(CandidTypeGraph &g) {
  CandidOpcode op;
  CandidKind k;
  if (CandidTypeGraph::prim_kind(m_datatype_opcode, k))
    return m_datatype_opcode;
  if (CandidTypeGraph::prim_kind(m_content_opcode, k)) {
    if (m_datatype_opcode == op.Vec) return g.vec(m_content_opcode);
    if (m_datatype_opcode == op.Opt) return g.opt(m_content_opcode);
  }
  ICPP_HOOKS::trap("ERROR: decoding a " + m_datatype_textual +
                   " of this kind is not supported");
  return 0;
}

void CandidTypeRoot::bind(const CandidValue &v) {
  ICPP_HOOKS::trap("ERROR: decoding a " + m_datatype_textual +
                   " of this kind is not supported");
}

void CandidTypeRoot::bind_push_back(const CandidValue &e) {
  ICPP_HOOKS::trap("ERROR: a CandidTypeVecRecord field must be a vec of a "
                   "primitive type, not a " +
                   m_datatype_textual);
}

void CandidTypeRoot::encode_M() {
  ICPP_HOOKS::trap("ERROR: encode_M not implemented for " + m_datatype_textual);
}

// https://github.com/dfinity/candid/blob/master/spec/Candid.md#records
uint32_t CandidTypeRoot::idl_hash(const std::string &s) {
  uint32_t hash{};

  // hash(id) = ( Sum_(i=0..k) utf8(id)[i] * 223^(k-i) ) mod 2^32 where k = |utf8(id)|-1
  hash = 0;
  uint64_t p = 4'294'967'296ULL; // std::pow(2,32)
  for (int c : s) {
    hash = (hash * 223 + c) % p;
  }

  return hash;
}

template <typename Derived> CandidTypeBase<Derived>::CandidTypeBase() {}

template <typename Derived> CandidType CandidTypeBase<Derived>::toCandidType() {
  // static_cast to the derived type
  return static_cast<Derived &>(*this);
}

// Explicit instantiations, to force the compiler to generate code for the methods of CandidTypeBase specialized for CandidTypeXXX
template class CandidTypeBase<CandidTypeBool>;
template class CandidTypeBase<CandidTypeEmpty>;
template class CandidTypeBase<CandidTypeFloat32>;
template class CandidTypeBase<CandidTypeFloat64>;
template class CandidTypeBase<CandidTypeInt>;
template class CandidTypeBase<CandidTypeInt8>;
template class CandidTypeBase<CandidTypeInt16>;
template class CandidTypeBase<CandidTypeInt32>;
template class CandidTypeBase<CandidTypeInt64>;
template class CandidTypeBase<CandidTypeNat>;
template class CandidTypeBase<CandidTypeNat8>;
template class CandidTypeBase<CandidTypeNat16>;
template class CandidTypeBase<CandidTypeNat32>;
template class CandidTypeBase<CandidTypeNat64>;
template class CandidTypeBase<CandidTypeNull>;
template class CandidTypeBase<CandidTypeOptNull>;
template class CandidTypeBase<CandidTypeOptBool>;
template class CandidTypeBase<CandidTypeOptFloat32>;
template class CandidTypeBase<CandidTypeOptFloat64>;
template class CandidTypeBase<CandidTypeOptInt>;
template class CandidTypeBase<CandidTypeOptInt8>;
template class CandidTypeBase<CandidTypeOptInt16>;
template class CandidTypeBase<CandidTypeOptInt32>;
template class CandidTypeBase<CandidTypeOptInt64>;
template class CandidTypeBase<CandidTypeOptNat>;
template class CandidTypeBase<CandidTypeOptNat8>;
template class CandidTypeBase<CandidTypeOptNat16>;
template class CandidTypeBase<CandidTypeOptNat32>;
template class CandidTypeBase<CandidTypeOptNat64>;
template class CandidTypeBase<CandidTypeOptPrincipal>;
template class CandidTypeBase<CandidTypeOptText>;
template class CandidTypeBase<CandidTypeOptRecord>;
template class CandidTypeBase<CandidTypeOptVariant>;
template class CandidTypeBase<CandidTypeOptVec>;
template class CandidTypeBase<CandidTypePrincipal>;
template class CandidTypeBase<CandidTypeRecord>;
template class CandidTypeBase<CandidTypeReserved>;
template class CandidTypeBase<CandidTypeText>;
template class CandidTypeBase<CandidTypeVariant>;
template class CandidTypeBase<CandidTypeVecBool>;
template class CandidTypeBase<CandidTypeVecFloat32>;
template class CandidTypeBase<CandidTypeVecFloat64>;
template class CandidTypeBase<CandidTypeVecInt>;
template class CandidTypeBase<CandidTypeVecInt8>;
template class CandidTypeBase<CandidTypeVecInt16>;
template class CandidTypeBase<CandidTypeVecInt32>;
template class CandidTypeBase<CandidTypeVecInt64>;
template class CandidTypeBase<CandidTypeVecNat>;
template class CandidTypeBase<CandidTypeVecNat8>;
template class CandidTypeBase<CandidTypeVecNat16>;
template class CandidTypeBase<CandidTypeVecNat32>;
template class CandidTypeBase<CandidTypeVecNat64>;
template class CandidTypeBase<CandidTypeVecPrincipal>;
template class CandidTypeBase<CandidTypeVecRecord>;
template class CandidTypeBase<CandidTypeVecText>;
template class CandidTypeBase<CandidTypeVecVariant>;
template class CandidTypeBase<CandidTypeVecBlob>;
template class CandidTypeBase<CandidTypeVecOf>;
template class CandidTypeBase<CandidTypeOptOf>;
template class CandidTypeBase<CandidTypeService>;
template class CandidTypeBase<CandidTypeFunc>;
template class CandidTypeBase<CandidTypeValue>;
