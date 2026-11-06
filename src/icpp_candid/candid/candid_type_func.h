// The class for the Candid Type: func - a reference to a method of a canister
//
// To encode one, e.g. the transform function of an HTTPS outcall:
//   CandidTypeFunc transform{canister_self, "transform"};
//   transform.append_arg(transform_args_record);
//   transform.append_ret(http_request_result_record);
//   transform.set_query();
//
// To decode one, give it the variables for the canister (as text) and the
// method name, and its expected type:
//   std::string canister, method;
//   CandidTypeFunc callback{&canister, &method};
//   callback.append_arg(CandidTypeText{});
//   callback.append_ret(CandidTypeNat{});
//   ic_api.from_wire(callback);

#pragma once

#include <cstring>
#include <memory>

#include "candid_type_principal.h"
#include "vec_bytes.h"

class CandidTypeFunc : public CandidTypeBase<CandidTypeFunc> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);
  void bind(const CandidValue &v);

  // Constructors
  // docs start: demo_candid_type_func
  CandidTypeFunc();
  CandidTypeFunc(const CandidTypePrincipal &canister,
                 const std::string &method);
  // For decoding: the canister's principal (as text) and the method name
  CandidTypeFunc(std::string *p_canister, std::string *p_method);
  // The types of the arguments and of the results, as CandidType instances
  void append_arg(CandidType arg_type);
  void append_ret(CandidType ret_type);
  // The annotation of the method
  void set_query();
  void set_composite_query();
  void set_oneway(); // docs end: demo_candid_type_func

  // Destructor
  ~CandidTypeFunc();

protected:
  void set_datatype();
  void encode_I();
  void encode_M();

  // Appends the type reference of a type to the type table encoding

  CandidTypePrincipal m_canister;
  std::string m_method;
  std::string *m_p_canister{nullptr};
  std::string *m_p_method{nullptr};
  std::vector<std::shared_ptr<CandidTypeRoot>> m_arg_ptrs;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_ret_ptrs;
  std::vector<uint8_t>
      m_annotations; // query = 1, oneway = 2, composite_query = 3
};
