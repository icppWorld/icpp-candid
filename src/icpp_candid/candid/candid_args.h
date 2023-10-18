// The class for the Candid Args

#pragma once

#include <cstring>
#include <memory>

#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "vec_bytes.h"

class CandidArgs {
public:
  // Constructors
  CandidArgs();

  // Destructor
  ~CandidArgs();

  void append(CandidType arg);
  std::vector<std::shared_ptr<CandidTypeRoot>> m_args_ptrs;
};