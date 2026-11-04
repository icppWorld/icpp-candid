// The class for the Candid Type: service - a reference to a canister
//
// To encode one, give it the canister and the types of its methods:
//   CandidTypeFunc greet;
//   greet.append_arg(CandidTypeText{});
//   greet.append_ret(CandidTypeText{});
//   greet.set_query();
//   CandidTypeService service{CandidTypePrincipal{"aaaaa-aa"}};
//   service.append_method("greet", greet);
//
// To decode one, give it the variable for the canister (as text), and its
// expected methods:
//   std::string canister;
//   CandidTypeService service{&canister};
//   service.append_method("greet", greet);
//   ic_api.from_wire(service);

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "candid_type_func.h"
#include "candid_type_principal.h"
#include "vec_bytes.h"

class CandidTypeService : public CandidTypeBase<CandidTypeService> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);
  void bind(const CandidValue &v);

  // Constructors
  CandidTypeService();
  CandidTypeService(const CandidTypePrincipal &canister);
  // For decoding: the canister's principal (as text)
  CandidTypeService(std::string *p_canister);
  // A method: its name, and its type as a CandidTypeFunc. Methods are kept
  // sorted by name, as the type table requires
  void append_method(const std::string &name, const CandidTypeFunc &method);

  // Destructor
  ~CandidTypeService();

protected:
  void set_datatype();
  void encode_T();
  void encode_I();
  void encode_M();

  CandidTypePrincipal m_canister;
  std::string *m_p_canister{nullptr};
  std::vector<std::pair<std::string, std::shared_ptr<CandidTypeFunc>>>
      m_methods;
};
