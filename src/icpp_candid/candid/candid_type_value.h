// The class for a Candid value of any type, recursive types included
//
// The type is a CandidTypeGraph and a type in it, the value a CandidValue.
// A recursive type is built with reserve() and define():
//
//   // type List = opt record { head : int; tail : List }
//   CandidTypeGraph g;
//   int list = g.reserve();
//   CandidGraphNode n;
//   n.kind = CandidKind::Opt;
//   n.inner = g.record({{CandidTypeGraph::idl_hash("head"),
//                        CandidTypeGraph::prim(CandidKind::Int)},
//                       {CandidTypeGraph::idl_hash("tail"), list}});
//   g.define(list, n);
//
// Encode: give the value, as a CandidValue (CandidTextValue::parse makes one
// from text). It traps if the value does not fit the type:
//   std::vector<CandidValue> v;
//   std::string err;
//   CandidTextValue::parse("(opt record {head = 1; tail = null})", g, {list},
//                          v, err);
//   ic_api.to_wire(CandidTypeValue{g, list, v[0]});
//
// Decode: give where the value goes:
//   CandidValue v;
//   ic_api.from_wire(CandidTypeValue{g, list, &v});
//
// Only the type, as the content of a CandidTypeOptOf or a CandidTypeVecOf:
//   CandidTypeOptOf{CandidTypeValue{g, list}}
//
// It works anywhere a CandidType does: as an argument, a record field, a
// variant case, in an opt and in a vec. In one message, a type used by
// several CandidTypeValues built from equal graphs gets one type table entry.

#pragma once

#include <memory>

#include "candid_type.h"
#include "candid_type_base.h"
#include "candid_type_graph.h"
#include "candid_value.h"

class CandidTypeValue : public CandidTypeBase<CandidTypeValue> {
public:
  // Decoding, with CandidDecoder: see CandidTypeRoot
  int graph_type(CandidTypeGraph &g);
  void bind(const CandidValue &v);

  // Encode: the value v of type t of g
  CandidTypeValue(const CandidTypeGraph &g, int t, const CandidValue &v);
  // Decode: the value of type t of g goes in *p_v
  CandidTypeValue(const CandidTypeGraph &g, int t, CandidValue *p_v);
  // Only the type t of g
  CandidTypeValue(const CandidTypeGraph &g, int t);

  ~CandidTypeValue();

  const CandidValue &get_v() const { return m_v; }

protected:
  void set_datatype();
  void encode_I();
  void encode_M();

  std::shared_ptr<const CandidTypeGraph> m_g;
  int m_t{0};
  CandidValue m_v;
  CandidValue *m_p_v{nullptr};

private:
  void initialize(const CandidTypeGraph &g, int t);
};
