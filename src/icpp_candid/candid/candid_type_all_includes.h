// https://github.com/dfinity/candid/blob/master/spec/Candid.md

#pragma once

// ------------------------------------------------------------------
// The include files

// The helper classes
// #include "candid_assert.h"
// #include "candid_opcode.h"
// #include "vec_bytes.h"

// The Candid Type base classes
// #include "candid_type_base.h"
// #include "candid_type_opt_base.h"
// #include "candid_type_vec_base.h"

// The Candid Type classes
// <primtype>
#include "candid_type_bool.h"
#include "candid_type_empty.h"
#include "candid_type_float32.h"
#include "candid_type_float64.h"
#include "candid_type_int.h"
#include "candid_type_int16.h"
#include "candid_type_int32.h"
#include "candid_type_int64.h"
#include "candid_type_int8.h"
#include "candid_type_nat.h"
#include "candid_type_nat16.h"
#include "candid_type_nat32.h"
#include "candid_type_nat64.h"
#include "candid_type_nat8.h"
#include "candid_type_null.h"
#include "candid_type_principal.h"
#include "candid_type_reserved.h"
#include "candid_type_text.h"

// <constype>
#include "candid_type_vec_bool.h"
#include "candid_type_vec_float32.h"
#include "candid_type_vec_float64.h"
#include "candid_type_vec_int.h"
#include "candid_type_vec_int16.h"
#include "candid_type_vec_int32.h"
#include "candid_type_vec_int64.h"
#include "candid_type_vec_int8.h"
#include "candid_type_vec_nat.h"
#include "candid_type_vec_nat16.h"
#include "candid_type_vec_nat32.h"
#include "candid_type_vec_nat64.h"
#include "candid_type_vec_nat8.h"
#include "candid_type_vec_principal.h"
#include "candid_type_vec_text.h"
// These types are not used for vectors
// #include "candid_type_vec_empty.h"
// #include "candid_type_vec_null.h"
// #include "candid_type_vec_reserved.h"

#include "candid_type_opt_bool.h"
#include "candid_type_opt_float32.h"
#include "candid_type_opt_float64.h"
#include "candid_type_opt_int.h"
#include "candid_type_opt_int16.h"
#include "candid_type_opt_int32.h"
#include "candid_type_opt_int64.h"
#include "candid_type_opt_int8.h"
#include "candid_type_opt_nat.h"
#include "candid_type_opt_nat16.h"
#include "candid_type_opt_nat32.h"
#include "candid_type_opt_nat64.h"
#include "candid_type_opt_nat8.h"
#include "candid_type_opt_principal.h"
#include "candid_type_opt_text.h"
// #include "candid_type_opt_vec.h"
// #include "candid_type_opt_record.h"
// These types are not used for opts
// #include "candid_type_opt_empty.h"
// #include "candid_type_opt_null.h"
// #include "candid_type_opt_reserved.h"

#include "candid_type_record.h"
#include "candid_type_variant.h"

// The Candid Type Table class
// #include "candid_type_table.h"

// The (De)Serialization classes
// #include "candid_deserialize.h"
// #include "candid_serialize.h"