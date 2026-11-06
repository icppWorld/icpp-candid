// Deprecated: since 7.1.0rc2 the type table is built at serialize time, by
// CandidEncoder, from the types of the arguments. This registry holds
// nothing; its methods are no-ops, kept so that code calling them, such as
// clear(), still compiles.
#pragma once

#include <cassert>
#include <vector>

#include <variant>

#include "candid_type.h"
#include "candid_type_all_includes.h"

struct CandidSerializeTypeTableEntry {
  VecBytes type_table_vec_bytes;
  size_t count;
};

class CandidSerializeTypeTableRegistry {
public:
  // Singleton pattern: ensures there's only one instance of this class.
  static CandidSerializeTypeTableRegistry &get_instance();

  // No-op
  void clear();

  // No-op: returns size_t(-1)
  size_t add_or_replace_type_table(size_t index, const VecBytes &T);

  // No-op
  void prune();

  // An empty VecBytes
  const VecBytes &get_type_table_vec_bytes(size_t index) const;

  // Empty
  std::vector<CandidSerializeTypeTableEntry> get_unique_type_tables() {
    return {};
  }

private:
  CandidSerializeTypeTableRegistry(); // Private constructor ensures it's not instantiated outside.
  ~CandidSerializeTypeTableRegistry(); // Although it's not necessary to declare the destructor private, it can be done for consistency.

  // Delete the copy & move constructors and assignment operators.
  CandidSerializeTypeTableRegistry(CandidSerializeTypeTableRegistry const &) =
      delete;
  void operator=(CandidSerializeTypeTableRegistry const &) = delete;
};