// Singleton to keep track of unique type tables during serialization
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

  // Clears the registry's data.
  void clear();

  /**
 *
 * Stores a copy of T in the registry, using this logic: 
 * 
 * - If the provided index is -1, a new type table is added, and its index is returned.
 * - If the provided index corresponds to a type table with a reference count of 1, the type table at that index is replaced in-place.
 * - If the provided index corresponds to a type table with a reference count greater than 1, the count is decremented and a new type table is added.
 *
 * @param index The index of the type table to replace, or -1 if adding a new type table.
 * @param T The type table VecBytes to add or replace with.
 * @return The index of the newly added or replaced type table.
 */
  size_t add_or_replace_type_table(size_t index, const VecBytes &T);

  /**
 * Prunes entries with a count of 0 from the end of the table. 
 * Stops pruning once an entry with a count greater than 0 is encountered.
 */
  void prune();

  // Get a type table based on its index.
  const VecBytes &get_type_table_vec_bytes(size_t index) const;

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

  // Add a type table if it's unique.
  // Returns the index of the type table (either newly added or pre-existing).
  size_t add_type_table(const VecBytes &T);

  // Remove a type table stored at index i if its count drops to zero and it is last in list
  void decrement_count_and_prune(size_t index);

  // Replace a type table stored at index i with a new one.
  void replace_type_table(size_t index, const VecBytes &new_table);
};