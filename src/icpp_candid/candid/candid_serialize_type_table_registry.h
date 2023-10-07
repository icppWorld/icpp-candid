// Singleton to keep track of unique type tables during serialization
#pragma once

#include <cassert>
#include <vector>

#include <variant>

#include "candid_type.h"
#include "candid_type_all_includes.h"

struct CandidSerializeTypeTableEntry {
  VecBytes type_table_bytes;
  size_t count;
};

class CandidSerializeTypeTableRegistry {
public:
  // Singleton pattern: ensures there's only one instance of this class.
  static CandidSerializeTypeTableRegistry &get_instance();

  // Clears the registry's data.
  void clear();

  // Add a type table if it's unique.
  // Returns the index of the type table (either newly added or pre-existing).
  __uint128_t add_type_table(const VecBytes &T);

  // Remove a type table if its count drops to zero.
  void remove_type_table(const VecBytes &T);

  // Get a type table based on its index.
  const VecBytes &get_type_table(size_t index) const;

  // Get the unique type tables in the registry
  std::vector<CandidSerializeTypeTableEntry> get_unique_type_tables() {
    return m_unique_type_tables;
  }

private:
  CandidSerializeTypeTableRegistry(); // Private constructor ensures it's not instantiated outside.
  ~CandidSerializeTypeTableRegistry(); // Although it's not necessary to declare the destructor private, it can be done for consistency.

  // Contains only unique type tables.
  std::vector<CandidSerializeTypeTableEntry> m_unique_type_tables;

  // Delete the copy & move constructors and assignment operators.
  CandidSerializeTypeTableRegistry(CandidSerializeTypeTableRegistry const &) =
      delete;
  void operator=(CandidSerializeTypeTableRegistry const &) = delete;
};