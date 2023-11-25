// https://github.com/dfinity/candid/blob/master/spec/Candid.md

#pragma once

// ------------------------------------------------------------------
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

#include "candid_type_record.h"
#include "candid_type_variant.h"

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
#include "candid_type_vec_record.h"
#include "candid_type_vec_text.h"
// #include "candid_type_vec_variant.h"
// #include "candid_type_vec_vec.h"
// These types are not used for vectors
// #include "candid_type_vec_empty.h"
// #include "candid_type_vec_null.h"
// #include "candid_type_vec_reserved.h"

#include "candid_type_opt_null.h"
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
// Opt of <constype>
#include "candid_type_opt_record.h"
#include "candid_type_opt_variant.h"
#include "candid_type_opt_vec.h"
// These types are not used for opts
// #include "candid_type_opt_empty.h"
// #include "candid_type_opt_reserved.h"

#include "candid_args.h"