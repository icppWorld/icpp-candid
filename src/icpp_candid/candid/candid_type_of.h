// Helpers for the classes that hold CandidType objects of any type:
// CandidTypeVecOf and CandidTypeOptOf. Include after candid_type_all_includes.h

#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

namespace candid_type_of {

// A CandidType as a pointer to its base class
inline std::shared_ptr<CandidTypeRoot> root_ptr(CandidType c) {
  return std::visit(
      [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
        auto derivedPtr = std::make_shared<std::decay_t<decltype(arg_)>>(
            std::forward<decltype(arg_)>(arg_));
        return std::static_pointer_cast<CandidTypeRoot>(derivedPtr);
      },
      c);
}

// True if a and b encode the same type
inline bool same_type(const std::shared_ptr<CandidTypeRoot> &a,
                      const std::shared_ptr<CandidTypeRoot> &b) {
  CandidTypeGraph g;
  int ta = a->graph_type(g);
  int tb = b->graph_type(g);
  return CandidTypeGraph::same(g, ta, g, tb);
}

} // namespace candid_type_of
