// The class for the Candid Args

#include "candid_args.h"

CandidArgs::CandidArgs() {}

CandidArgs::~CandidArgs() {}

// Store the shared pointer to a CandidTypeBase class
void CandidArgs::append(CandidType arg) {
  m_args_ptrs.push_back(std::visit([](auto&& arg_) -> std::shared_ptr<CandidTypeBase> {
    return std::make_shared<std::decay_t<decltype(arg_)>>(arg_);
  }, arg));
}