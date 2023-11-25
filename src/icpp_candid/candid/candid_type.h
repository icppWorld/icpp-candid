// https://github.com/dfinity/candid/blob/master/spec/Candid.md

#pragma once

// #include <optional>
#include <variant>

// ------------------------------------------------------------------
// Forward declarations

// The helper classes
class CandidAssert;
class Opcode;
class VecBytes;

// TODO: REEMOVE
// The Candid Type base classes
// template <typename Derived> class CandidTypeBase;
// template <typename Derived> class CandidTypeBase;
// template <typename Derived> class CandidTypeBase;

// #include "candid_type_base.h"

// The Candid Type classes
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#types
// <primtype>
class CandidTypeNull;
class CandidTypeEmpty;
class CandidTypeReserved;
class CandidTypeBool;
class CandidTypeFloat32;
class CandidTypeFloat64;
class CandidTypeInt;
class CandidTypeNat;
class CandidTypeNat8;
class CandidTypeNat16;
class CandidTypeNat32;
class CandidTypeNat64;
class CandidTypeInt8;
class CandidTypeInt16;
class CandidTypeInt32;
class CandidTypeInt64;
class CandidTypeText;
class CandidTypePrincipal;

// <constype>
class CandidTypeVecBool;
class CandidTypeVecFloat32;
class CandidTypeVecFloat64;
class CandidTypeVecInt;
class CandidTypeVecNat;
class CandidTypeVecNat8;
class CandidTypeVecNat16;
class CandidTypeVecNat32;
class CandidTypeVecNat64;
class CandidTypeVecInt8;
class CandidTypeVecInt16;
class CandidTypeVecInt32;
class CandidTypeVecInt64;
class CandidTypeVecText;
class CandidTypeVecPrincipal;
class CandidTypeVecRecord;
// These types are not used for vecs
// class CandidTypeVecNull;
// class CandidTypeVecEmpty;
// class CandidTypeVecReserved;

class CandidTypeOptNull;
class CandidTypeOptBool;
class CandidTypeOptFloat32;
class CandidTypeOptFloat64;
class CandidTypeOptInt;
class CandidTypeOptNat;
class CandidTypeOptNat8;
class CandidTypeOptNat16;
class CandidTypeOptNat32;
class CandidTypeOptNat64;
class CandidTypeOptInt8;
class CandidTypeOptInt16;
class CandidTypeOptInt32;
class CandidTypeOptInt64;
class CandidTypeOptText;
class CandidTypeOptPrincipal;
class CandidTypeOptVec;
class CandidTypeOptRecord;
class CandidTypeOptVariant;
// These types are not used for opts
// class CandidTypeOptEmpty;
// class CandidTypeOptReserved;

class CandidTypeRecord;
class CandidTypeVariant;

// The Candid Type Table class
class CandidTypeTable;

// The (De)Serialization classes
class CandidDeserialize;
class CandidSerialize;

// The Candid Args class
class CandidArgs;

// The std::variant alias
using CandidType = std::variant<
    CandidTypeNull, CandidTypeEmpty, CandidTypeReserved, CandidTypeBool,
    CandidTypeFloat32, CandidTypeFloat64, CandidTypeInt, CandidTypeNat,
    CandidTypeNat8, CandidTypeNat16, CandidTypeNat32, CandidTypeNat64,
    CandidTypeInt8, CandidTypeInt16, CandidTypeInt32, CandidTypeInt64,
    CandidTypeText, CandidTypePrincipal, CandidTypeVecBool,
    CandidTypeVecFloat32, CandidTypeVecFloat64, CandidTypeVecInt,
    CandidTypeVecNat, CandidTypeVecNat8, CandidTypeVecNat16, CandidTypeVecNat32,
    CandidTypeVecNat64, CandidTypeVecInt8, CandidTypeVecInt16,
    CandidTypeVecInt32, CandidTypeVecInt64, CandidTypeVecText,
    CandidTypeVecPrincipal, CandidTypeVecRecord, CandidTypeOptNull,
    CandidTypeOptBool, CandidTypeOptFloat32, CandidTypeOptFloat64,
    CandidTypeOptInt, CandidTypeOptNat, CandidTypeOptNat8, CandidTypeOptNat16,
    CandidTypeOptNat32, CandidTypeOptNat64, CandidTypeOptInt8,
    CandidTypeOptInt16, CandidTypeOptInt32, CandidTypeOptInt64,
    CandidTypeOptText, CandidTypeOptPrincipal, CandidTypeOptRecord,
    CandidTypeOptVariant, CandidTypeOptVec, CandidTypeRecord,
    CandidTypeVariant>;