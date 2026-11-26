// The class for the Candid Args

#include "candid_args.h"

#include <utility>

#include "candid_type_of.h"

CandidArgs::CandidArgs() {}

// Store the shared pointer to a CandidTypeRoot class
void CandidArgs::append(CandidType arg) {
  m_args_ptrs.push_back(candid_type_of::root_ptr(std::move(arg)));
}