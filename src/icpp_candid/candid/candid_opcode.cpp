// The Candid type Opcodes and helper methods

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>

#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "candid_type_table.h"
#include "icpp_hooks.h"

CandidOpcode::CandidOpcode() {}

CandidOpcode::~CandidOpcode() {}

bool CandidOpcode::is_primtype(const int &t) {
  //     Null    Bool       Nat        Int        Nat8       Nat16
  if (t == -1 || t == -2 || t == -3 || t == -4 || t == -5 || t == -6 ||
      // Nat32   Nat64      Int8       Int16       Int32       Int64
      t == -7 || t == -8 || t == -9 || t == -10 || t == -11 || t == -12 ||
      // Float32  Float64     Text        Reserved    Empty       Principal
      t == -13 || t == -14 || t == -15 || t == -16 || t == -17 || t == -24)
    return true;
  return false;
}

bool CandidOpcode::is_constype(const int &t) {
  //  Opt         Vec         Record      Variant
  if (t == -18 || t == -19 || t == -20 || t == -21) return true;
  return false;
}

bool CandidOpcode::is_reftype(const int &t) {
  //  Func        Service
  if (t == -22 || t == -23) return true;
  return false;
}

std::string CandidOpcode::name_from_opcode(int opcode) {
  // This function uses a hash map to map the opcodes to their corresponding variable names.
  // It first checks if the given opcode exists in the map.
  // If it does, the function returns the associated variable name; otherwise, it returns "Unknown".
  std::unordered_map<int, std::string> names{
      {-1, "Null"},     {-2, "Bool"},     {-3, "Nat"},      {-4, "Int"},
      {-5, "Nat8"},     {-6, "Nat16"},    {-7, "Nat32"},    {-8, "Nat64"},
      {-9, "Int8"},     {-10, "Int16"},   {-11, "Int32"},   {-12, "Int64"},
      {-13, "Float32"}, {-14, "Float64"}, {-15, "Text"},    {-16, "Reserved"},
      {-17, "Empty"},   {-18, "Opt"},     {-19, "Vec"},     {-20, "Record"},
      {-21, "Variant"}, {-22, "Func"},    {-23, "Service"}, {-24, "Principal"}};

  if (names.count(opcode)) {
    return names[opcode];
  } else if (opcode >= 0) {
    return "type table index";
  } else {
    return "Unknown";
  }
}

std::shared_ptr<CandidTypeRoot>
CandidOpcode::candid_type_from_opcode(int opcode) {
  if (opcode == Null) {
    return std::make_shared<CandidTypeNull>();
  } else if (opcode == Empty) {
    return std::make_shared<CandidTypeEmpty>();
  } else if (opcode == Reserved) {
    return std::make_shared<CandidTypeReserved>();
  } else if (opcode == Bool) {
    return std::make_shared<CandidTypeBool>();
  } else if (opcode == Float32) {
    return std::make_shared<CandidTypeFloat32>();
  } else if (opcode == Float64) {
    return std::make_shared<CandidTypeFloat64>();
  } else if (opcode == Int) {
    return std::make_shared<CandidTypeInt>();
  } else if (opcode == Nat) {
    return std::make_shared<CandidTypeNat>();
  } else if (opcode == Nat8) {
    return std::make_shared<CandidTypeNat8>();
  } else if (opcode == Nat16) {
    return std::make_shared<CandidTypeNat16>();
  } else if (opcode == Nat32) {
    return std::make_shared<CandidTypeNat32>();
  } else if (opcode == Nat64) {
    return std::make_shared<CandidTypeNat64>();
  } else if (opcode == Int8) {
    return std::make_shared<CandidTypeInt8>();
  } else if (opcode == Int16) {
    return std::make_shared<CandidTypeInt16>();
  } else if (opcode == Int32) {
    return std::make_shared<CandidTypeInt32>();
  } else if (opcode == Int64) {
    return std::make_shared<CandidTypeInt64>();
  } else if (opcode == Text) {
    return std::make_shared<CandidTypeText>();
  } else if (opcode == Principal) {
    return std::make_shared<CandidTypePrincipal>();
  } else if (opcode == Vec) {
    std::string msg;
    msg.append(
        "ERROR: NOT SUPPORTED FOR VEC. (use candid_vec_type_from_opcode)");
    msg.append("       opcode = " + std::to_string(opcode) + " (" +
               name_from_opcode(opcode) + ")");
    msg.append("      " + std::string(__func__));
    ICPP_HOOKS::trap(msg);
    return std::make_shared<CandidTypeNull>(); // unreachable
  } else if (opcode == Opt) {
    std::string msg;
    msg.append(
        "ERROR: NOT SUPPORTED FOR OPT. (use candid_opt_type_from_opcode)");
    msg.append("       opcode = " + std::to_string(opcode) + " (" +
               name_from_opcode(opcode) + ")");
    msg.append("      " + std::string(__func__));
    ICPP_HOOKS::trap(msg);
    return std::make_shared<CandidTypeNull>(); // unreachable
  } else if (opcode == Record) {
    return std::make_shared<CandidTypeRecord>();
  } else if (opcode == Variant) {
    return std::make_shared<CandidTypeVariant>();
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED FOR THIS OPCODE.");
    msg.append("       opcode = " + std::to_string(opcode) + " (" +
               name_from_opcode(opcode) + ")");
    msg.append("      " + std::string(__func__));
    ICPP_HOOKS::trap(msg);
    return std::make_shared<CandidTypeNull>(); // unreachable
  }
}

std::shared_ptr<CandidTypeRoot> CandidOpcode::candid_type_vec_from_opcode(
    int content_opcode, CandidTypeTable *p_content_type_table) {
  // if (content_opcode == Null) {
  // return std::make_shared<CandidTypeVecNull>();
  // } else if (content_opcode == Empty) {
  // return std::make_shared<CandidTypeVecEmpty>();
  // } else if (content_opcode == Reserved) {
  // return std::make_shared<CandidTypeVecReserved>();
  // } else
  if (content_opcode == Bool) {
    return std::make_shared<CandidTypeVecBool>();
  } else if (content_opcode == Float32) {
    return std::make_shared<CandidTypeVecFloat32>();
  } else if (content_opcode == Float64) {
    return std::make_shared<CandidTypeVecFloat64>();
  } else if (content_opcode == Int) {
    return std::make_shared<CandidTypeVecInt>();
  } else if (content_opcode == Nat) {
    return std::make_shared<CandidTypeVecNat>();
  } else if (content_opcode == Nat8) {
    return std::make_shared<CandidTypeVecNat8>();
  } else if (content_opcode == Nat16) {
    return std::make_shared<CandidTypeVecNat16>();
  } else if (content_opcode == Nat32) {
    return std::make_shared<CandidTypeVecNat32>();
  } else if (content_opcode == Nat64) {
    return std::make_shared<CandidTypeVecNat64>();
  } else if (content_opcode == Int8) {
    return std::make_shared<CandidTypeVecInt8>();
  } else if (content_opcode == Int16) {
    return std::make_shared<CandidTypeVecInt16>();
  } else if (content_opcode == Int32) {
    return std::make_shared<CandidTypeVecInt32>();
  } else if (content_opcode == Int64) {
    return std::make_shared<CandidTypeVecInt64>();
  } else if (content_opcode == Text) {
    return std::make_shared<CandidTypeVecText>();
  } else if (content_opcode == Principal) {
    return std::make_shared<CandidTypeVecPrincipal>();
  } else if (content_opcode == Record) {
    if (p_content_type_table) {
      std::shared_ptr<CandidTypeRoot> p_wire_record =
          p_content_type_table->get_p_wire();
      return std::make_shared<CandidTypeVecRecord>(p_wire_record);
    } else {
      return std::make_shared<CandidTypeVecRecord>();
    }
    // } else if (content_opcode == Variant) {
    //   if (p_content_type_table) {
    //     std::shared_ptr<CandidTypeRoot> p_wire =
    //         p_content_type_table->get_p_wire();
    //     return std::make_shared<CandidTypeVecVariant>(p_wire);
    //   } else {
    //     return std::make_shared<CandidTypeVecVariant>();
    //   }
    // } else if (content_opcode == Vec) {
    //   if (p_content_type_table) {
    //     std::shared_ptr<CandidTypeRoot> p_wire =
    //         p_content_type_table->get_p_wire();
    //     return std::make_shared<CandidTypeVecVec>(p_wire);
    //   } else {
    //     return std::make_shared<CandidTypeVecVec>();
    //   }
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED CandidTypeVecXXX.");
    msg.append("       for content_opcode = " + std::to_string(content_opcode) +
               " (" + name_from_opcode(content_opcode) + ")");
    msg.append("      " + std::string(__func__));
    ICPP_HOOKS::trap(msg);
    return std::make_shared<CandidTypeVecBool>(); // avoid compiler warning
  }
}

std::shared_ptr<CandidTypeRoot> CandidOpcode::candid_type_opt_from_opcode(
    int content_opcode, CandidTypeTable *p_content_type_table) {
  if (content_opcode == Null) {
    return std::make_shared<CandidTypeOptNull>();
    // } else if (content_opcode == Empty) {
    // return std::make_shared<CandidTypeOptEmpty>();
    // } else if (content_opcode == Reserved) {
    // return std::make_shared<CandidTypeOptReserved>();
  } else if (content_opcode == Bool) {
    return std::make_shared<CandidTypeOptBool>();
  } else if (content_opcode == Float32) {
    return std::make_shared<CandidTypeOptFloat32>();
  } else if (content_opcode == Float64) {
    return std::make_shared<CandidTypeOptFloat64>();
  } else if (content_opcode == Int) {
    return std::make_shared<CandidTypeOptInt>();
  } else if (content_opcode == Nat) {
    return std::make_shared<CandidTypeOptNat>();
  } else if (content_opcode == Nat8) {
    return std::make_shared<CandidTypeOptNat8>();
  } else if (content_opcode == Nat16) {
    return std::make_shared<CandidTypeOptNat16>();
  } else if (content_opcode == Nat32) {
    return std::make_shared<CandidTypeOptNat32>();
  } else if (content_opcode == Nat64) {
    return std::make_shared<CandidTypeOptNat64>();
  } else if (content_opcode == Int8) {
    return std::make_shared<CandidTypeOptInt8>();
  } else if (content_opcode == Int16) {
    return std::make_shared<CandidTypeOptInt16>();
  } else if (content_opcode == Int32) {
    return std::make_shared<CandidTypeOptInt32>();
  } else if (content_opcode == Int64) {
    return std::make_shared<CandidTypeOptInt64>();
  } else if (content_opcode == Text) {
    return std::make_shared<CandidTypeOptText>();
  } else if (content_opcode == Principal) {
    return std::make_shared<CandidTypeOptPrincipal>();
  } else if (content_opcode == Record) {
    if (p_content_type_table) {
      std::shared_ptr<CandidTypeRoot> p_wire =
          p_content_type_table->get_p_wire();
      return std::make_shared<CandidTypeOptRecord>(p_wire);
    } else {
      return std::make_shared<CandidTypeOptRecord>();
    }
  } else if (content_opcode == Variant) {
    if (p_content_type_table) {
      std::shared_ptr<CandidTypeRoot> p_wire =
          p_content_type_table->get_p_wire();
      return std::make_shared<CandidTypeOptVariant>(p_wire);
    } else {
      return std::make_shared<CandidTypeOptVariant>();
    }
  } else if (content_opcode == Vec) {
    if (p_content_type_table) {
      std::shared_ptr<CandidTypeRoot> p_wire =
          p_content_type_table->get_p_wire();
      return std::make_shared<CandidTypeOptVec>(p_wire);
    } else {
      return std::make_shared<CandidTypeOptVec>();
    }
  } else {
    std::string msg;
    msg.append("ERROR: NOT YET IMPLEMENTED CandidTypeOptXXX.");
    msg.append("       for content_opcode = " + std::to_string(content_opcode) +
               " (" + name_from_opcode(content_opcode) + ")");
    msg.append("      " + std::string(__func__));
    ICPP_HOOKS::trap(msg);
    return std::make_shared<CandidTypeOptNull>();
  }
}