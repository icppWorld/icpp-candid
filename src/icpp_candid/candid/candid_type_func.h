// The class for the Candid Type: func - a reference to a method of a canister
//
// For encoding only, e.g. the transform function of an HTTPS outcall:
//   CandidTypeFunc transform{canister_self, "transform"};
//   transform.append_arg(transform_args_record);
//   transform.append_ret(http_request_result_record);
//   transform.set_query();

#pragma once

#include <cstring>
#include <memory>

#include "candid_type_principal.h"
#include "vec_bytes.h"

class CandidTypeFunc : public CandidTypeBase<CandidTypeFunc> {
public:
  // Constructors
  // docs start: demo_candid_type_func
  CandidTypeFunc();
  CandidTypeFunc(const CandidTypePrincipal &canister,
                 const std::string &method);
  // The types of the arguments and of the results, as CandidType instances
  void append_arg(CandidType arg_type);
  void append_ret(CandidType ret_type);
  // The annotation of the method
  void set_query();
  void set_composite_query();
  void set_oneway(); // docs end: demo_candid_type_func

  // Destructor
  ~CandidTypeFunc();

  // Decoding a func is not supported
  bool decode_M(CandidDeserialize &de, VecBytes &B, __uint128_t &offset,
                std::string &parse_error);

protected:
  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  // Appends the type reference of a type to the type table encoding
  void append_type_reference(std::shared_ptr<CandidTypeRoot> p_type);

  CandidTypePrincipal m_canister;
  std::string m_method;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_arg_ptrs;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_ret_ptrs;
  std::vector<uint8_t>
      m_annotations; // query = 1, oneway = 2, composite_query = 3
};
