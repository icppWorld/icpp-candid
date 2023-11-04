// The class for the Candid Args

#include "candid_args.h"

CandidArgs::CandidArgs() {}

CandidArgs::~CandidArgs() {}

// Store the shared pointer to a CandidTypeRoot class
void CandidArgs::append(CandidType arg) {
  m_args_ptrs.push_back(std::visit(
      [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
        // Create shared_ptr for the derived type
        auto derivedPtr = std::make_shared<std::decay_t<decltype(arg_)>>(
            std::forward<decltype(arg_)>(arg_));

        // Cast it to a shared_ptr of the base type
        return std::static_pointer_cast<CandidTypeRoot>(derivedPtr);
      },
      arg));
  // This makes a copy
  // m_args_ptrs.push_back(std::visit(
  //     [](auto &&arg_) -> std::shared_ptr<CandidTypeRoot> {
  //       return std::make_shared<std::decay_t<decltype(arg_)>>(arg_);
  //     },
  //     arg));
}