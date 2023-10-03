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

// Add type table and get its unique index.
__uint128_t
CandidSerializeTypeTableRegistry::add_type_table(const VecBytes &T) {
  for (size_t i = 0; i < m_unique_type_tables.size(); ++i) {
    if (T == m_unique_type_tables[i].type_table_bytes) {
      // Type Table already exists, just increase the count (usage)
      m_unique_type_tables[i].count++;
      return i;
    }
  }
  m_unique_type_tables.push_back({T, 1}); // New entry with count of 1
  return m_unique_type_tables.size() - 1;
}

void CandidSerializeTypeTableRegistry::remove_type_table(const VecBytes &T) {
  // Return early if T is empty.
  if (T.size() == 0) {
    return;
  }

  for (size_t i = 0; i < m_unique_type_tables.size(); ++i) {
    if (T == m_unique_type_tables[i].type_table_bytes) {
      if (m_unique_type_tables[i].count > 0) m_unique_type_tables[i].count--;
      // We can safely erase it if count is zero and it's the last element in the vector
      if (m_unique_type_tables[i].count == 0 &&
          i == m_unique_type_tables.size() - 1) {
        m_unique_type_tables.erase(m_unique_type_tables.begin() + i);
      }
      return;
    }
  }
  // If you reach here, you tried to remove a type table that doesn't exist.
  assert(false && "Type table not found");
}

// Retrieve type table based on its index.
const VecBytes &
CandidSerializeTypeTableRegistry::get_type_table(size_t index) const {
  assert(index < m_unique_type_tables.size());
  return m_unique_type_tables[index].type_table_bytes;
}
