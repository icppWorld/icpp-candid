// The base class for all Candid Types

#include "candid_type.h"
#include "candid_type_all_includes.h"

#include "candid_opcode.h"
#include "candid_type_base.h"

#include <charconv>
#include <cmath>
#include <system_error>

#include "icpp_hooks.h"

CandidTypeRoot::CandidTypeRoot() {}
CandidTypeRoot::~CandidTypeRoot() {}

void CandidTypeRoot::trap_if_wrong_type_on_wire(
    const std::string &type_on_wire) {
  if (type_on_wire != m_datatype_textual) {
    std::string msg;
    msg.append("ERROR: wrong Candid type on wire.\n");
    msg.append("       Candid type on wire  = " + type_on_wire + "\n");
    msg.append("       Expected Candid type = " + m_datatype_textual);
    ICPP_HOOKS::trap(msg);
  };
}

// Virtual method to be implemented by all derived classes
CandidType CandidTypeRoot::toCandidType() {
  ICPP_HOOKS::trap("ERROR: toCandidType not implemented for " +
                   m_datatype_textual);
  return CandidTypeBool{false}; // Need to return something
}

// Virtual method to be implemented by the <comptype> CandidTypes
// Non <comptype> should not call this method.
bool CandidTypeRoot::decode_T(const VecBytes B, __uint128_t &offset,
                              std::string &parse_error) {
  ICPP_HOOKS::trap("ERROR: decode_T not implemented for " + m_datatype_textual);
  return true;
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

// Virtual method to be implemented by all CandidTypeVecXXX to push_back a value into the internal std::vector<T>
void CandidTypeRoot::push_back_value(CandidTypeRoot &value) {
  ICPP_HOOKS::trap("ERROR: set_content_type not implemented for " +
                   m_datatype_textual);
}

// Virtual method to be implemented by all CandidTypes to support deserialization
bool CandidTypeRoot::decode_M(CandidDeserialize &de, VecBytes B,
                              __uint128_t &offset, std::string &parse_error) {
  ICPP_HOOKS::trap("ERROR: decode_M not implemented for " + m_datatype_textual);
  return true;
}

// Virtual method to be implemented by Record (and perhaps other constypes)  to support deserialization
void CandidTypeRoot::set_fields_wire(
    std::shared_ptr<CandidTypeRoot> p_from_wire) {
  ICPP_HOOKS::trap("ERROR: set_fields_wire not implemented for " +
                   m_datatype_textual);
}

void CandidTypeRoot::finish_decode_T(CandidDeserialize &de) {
  ICPP_HOOKS::trap("ERROR: finish_decode_T not implemented for " +
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
  uint64_t p = 4294967296; // std::pow(2,32)
  for (int c : s) {
    hash = (hash * 223 + c) % p;
  }

  return hash;
}

template <typename Derived> CandidTypeBase<Derived>::CandidTypeBase() {}

template <typename Derived> CandidTypeBase<Derived>::~CandidTypeBase() {}

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