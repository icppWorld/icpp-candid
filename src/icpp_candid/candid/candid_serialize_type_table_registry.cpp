// Singleton to keep track of unique type tables during serialization

#include "candid_serialize_type_table_registry.h"

// Define the static method for Singleton instance access.
CandidSerializeTypeTableRegistry &
CandidSerializeTypeTableRegistry::get_instance() {
  static CandidSerializeTypeTableRegistry instance;
  return instance;
}

// Constructor (defined here even if it does nothing).
CandidSerializeTypeTableRegistry::CandidSerializeTypeTableRegistry() = default;

// Destructor (if you need to add any cleanup logic in the future).
CandidSerializeTypeTableRegistry::~CandidSerializeTypeTableRegistry() = default;

void CandidSerializeTypeTableRegistry::clear() { m_unique_type_tables.clear(); }

size_t
CandidSerializeTypeTableRegistry::add_or_replace_type_table(size_t index,
                                                            const VecBytes &T) {
  // Check for an invalid index (excluding the sentinel value -1).
  if (index != -1 && index >= m_unique_type_tables.size()) {
    ICPP_HOOKS::trap(
        "ERROR: Invalid type table index provided to add_or_replace_type_table.");
  }

  if (index == -1) {
    // The type table has not been added yet.
    return add_type_table(T);
  } else if (m_unique_type_tables[index].count == 1) {
    // Only one reference to the type table exists, so we can replace it.
    replace_type_table(index, T);
    return index; // Return the same index as the type table was replaced in-place.
  } else {
    // More than one reference to the type table.
    // Don't replace; instead, remove the type table and add a new type table.
    decrement_count_and_prune(index);
    return add_type_table(T);
  }
}

// Add type table and get its unique index.
size_t CandidSerializeTypeTableRegistry::add_type_table(const VecBytes &T) {
  prune(); // Prune before we add, just in case we can remove some entries

  for (size_t i = 0; i < m_unique_type_tables.size(); ++i) {
    if (T == m_unique_type_tables[i].type_table_vec_bytes) {
      // Type Table already exists, just increase the count (usage)
      m_unique_type_tables[i].count++;
      return i;
    }
  }

  m_unique_type_tables.push_back({T, 1}); // New entry with count of 1
  return m_unique_type_tables.size() - 1;
}

// Remove a type table stored at index i if its count drops to zero and it is last in list
void CandidSerializeTypeTableRegistry::decrement_count_and_prune(size_t index) {
  // Ensure index is valid.
  if (index >= m_unique_type_tables.size())
    ICPP_HOOKS::trap(std::string(__func__) +
                     ": ERROR - index is larger than size.");

  if (m_unique_type_tables[index].count > 0)
    m_unique_type_tables[index].count--;

  prune();
}

/**
 * Prunes entries with a count of 0 from the end of the table. 
 * Stops pruning once an entry with a count greater than 0 is encountered.
 */
void CandidSerializeTypeTableRegistry::prune() {
  for (ssize_t i = m_unique_type_tables.size() - 1; i >= 0; --i) {
    if (m_unique_type_tables[i].count == 0) {
      m_unique_type_tables.erase(m_unique_type_tables.begin() + i);
    } else {
      break; // Exit the loop once last item has a count > 0
             // We cannot remove previous items, because that would reorder
    }
  }
}

void CandidSerializeTypeTableRegistry::replace_type_table(
    size_t index, const VecBytes &new_table) {
  // Ensure index is valid.
  if (index >= m_unique_type_tables.size())
    ICPP_HOOKS::trap(std::string(__func__) +
                     ": ERROR - index is larger than size.");

  // Replace the type table at the given index.
  m_unique_type_tables[index].type_table_vec_bytes = new_table;
}

// Retrieve type table based on its index.
const VecBytes &
CandidSerializeTypeTableRegistry::get_type_table_vec_bytes(size_t index) const {
  assert(index < m_unique_type_tables.size());
  return m_unique_type_tables[index].type_table_vec_bytes;
}
