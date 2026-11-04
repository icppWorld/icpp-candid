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
  if (a->get_T().size() > 0 || b->get_T().size() > 0) {
    return a->get_T().size() > 0 && b->get_T().size() > 0 &&
           a->get_type_table_index() == b->get_type_table_index();
  }
  return a->get_datatype_opcode() == b->get_datatype_opcode();
}

} // namespace candid_type_of
