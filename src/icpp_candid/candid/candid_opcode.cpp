// The Candid type Opcodes and helper methods

#include <string>
#include <unordered_map>
#include <variant>
#include <memory>

#include "candid_opcode.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
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
