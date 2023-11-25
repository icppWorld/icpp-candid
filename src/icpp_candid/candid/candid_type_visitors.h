// std::visit helper functions for std::variant CandidType

#pragma once

#include <optional>
#include <variant>

#include "candid_type_all_includes.h"

// ---------------------------------------------------------
// To check if a variable is a CandidType
template <typename T> bool is_candid_type(const CandidType &c) {
  return holds_alternative<T>(c);
}

// ---------------------------------------------------------
// Enable sorting of a vector of CandidTypes
// REMOVED: it does not compile to wasm
// bool operator<(CandidType &lhs, CandidType &rhs) {
//   auto get_opcode = [](auto &val) -> int { return val.get_datatype_opcode(); };

//   int lhs_opcode = std::visit(get_opcode, lhs);
//   int rhs_opcode = std::visit(get_opcode, rhs);

//   return lhs_opcode < rhs_opcode;
// }

// ---------------------------------------------------------
// These make copies... See candid_args.cpp for an alternative
// Utility lambda to obtain CandidTypeRoot pointer from CandidType. A non-const & const version.
// auto get_candid_type_root_ptr = [](CandidType &candidType) -> CandidTypeRoot * {
//   return std::visit([](auto &&arg) -> CandidTypeRoot * { return &arg; },
//                     candidType);
// };
// auto get_candid_type_root_ptr_const =
//     [](const CandidType &candidType) -> const CandidTypeRoot * {
//   return std::visit([](auto &&arg) -> const CandidTypeRoot * { return &arg; },
//                     candidType);
// };

// ---------------------------------------------------------
// A visitor to the index of a CandidTypeVecXXX, returning it's value as a CandidType

template <typename T_CandidTypeVec, typename T_CandidType>
T_CandidType get_vec_value(T_CandidTypeVec &c, size_t index) {
  if (c.get_v().size() > 0 && index < c.get_v().size()) {
    return T_CandidType{c.get_v()[index]};
  }
  T_CandidType c_default;
  return c_default;
}

struct GetCandidTypeVecIndexVisitor {
  size_t idx;

  GetCandidTypeVecIndexVisitor(size_t index) : idx(index) {}

  auto operator()(CandidTypeVecBool &c) -> CandidType {
    return get_vec_value<CandidTypeVecBool, CandidTypeBool>(c, idx);
  }
  auto operator()(CandidTypeVecFloat32 &c) -> CandidType {
    return get_vec_value<CandidTypeVecFloat32, CandidTypeFloat32>(c, idx);
  }
  auto operator()(CandidTypeVecFloat64 &c) -> CandidType {
    return get_vec_value<CandidTypeVecFloat64, CandidTypeFloat64>(c, idx);
  }
  auto operator()(CandidTypeVecInt &c) -> CandidType {
    return get_vec_value<CandidTypeVecInt, CandidTypeInt>(c, idx);
  }
  auto operator()(CandidTypeVecInt8 &c) -> CandidType {
    return get_vec_value<CandidTypeVecInt8, CandidTypeInt8>(c, idx);
  }
  auto operator()(CandidTypeVecInt16 &c) -> CandidType {
    return get_vec_value<CandidTypeVecInt16, CandidTypeInt16>(c, idx);
  }
  auto operator()(CandidTypeVecInt32 &c) -> CandidType {
    return get_vec_value<CandidTypeVecInt32, CandidTypeInt32>(c, idx);
  }
  auto operator()(CandidTypeVecInt64 &c) -> CandidType {
    return get_vec_value<CandidTypeVecInt64, CandidTypeInt64>(c, idx);
  }
  auto operator()(CandidTypeVecNat &c) -> CandidType {
    return get_vec_value<CandidTypeVecNat, CandidTypeNat>(c, idx);
  }
  auto operator()(CandidTypeVecNat8 &c) -> CandidType {
    return get_vec_value<CandidTypeVecNat8, CandidTypeNat8>(c, idx);
  }
  auto operator()(CandidTypeVecNat16 &c) -> CandidType {
    return get_vec_value<CandidTypeVecNat16, CandidTypeNat16>(c, idx);
  }
  auto operator()(CandidTypeVecNat32 &c) -> CandidType {
    return get_vec_value<CandidTypeVecNat32, CandidTypeNat32>(c, idx);
  }
  auto operator()(CandidTypeVecNat64 &c) -> CandidType {
    return get_vec_value<CandidTypeVecNat64, CandidTypeNat64>(c, idx);
  }
  auto operator()(CandidTypeVecPrincipal &c) -> CandidType {
    return get_vec_value<CandidTypeVecPrincipal, CandidTypePrincipal>(c, idx);
  }
  // auto operator()(CandidTypeVecRecord &c) -> CandidType {
  //   return get_vec_value<CandidTypeVecRecord, CandidTypeRecord>(c, idx);
  // }
  auto operator()(CandidTypeVecText &c) -> CandidType {
    return get_vec_value<CandidTypeVecText, CandidTypeText>(c, idx);
  }

  // Default handler for types that do not support get_v()
  template <typename T> auto operator()(T &c) -> CandidType {
    std::string msg;
    msg.append(
        "ERROR: GetCandidTypeVecIndexVisitor not implemented for CandidType = ");
    msg.append("'" + c.get_content_textual() + "'");
    ICPP_HOOKS::trap(msg);
    return CandidTypeNull{}; // Have to return something to compile
  }
};
