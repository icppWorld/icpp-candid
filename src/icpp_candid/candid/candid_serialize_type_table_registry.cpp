// Deprecated: since 7.1.0rc2 the type table is built at serialize time, by
// CandidEncoder, and this registry holds nothing. Its methods are kept, as
// no-ops, so code that calls them still compiles.

#include "candid_serialize_type_table_registry.h"

CandidSerializeTypeTableRegistry &
CandidSerializeTypeTableRegistry::get_instance() {
  static CandidSerializeTypeTableRegistry instance;
  return instance;
}

CandidSerializeTypeTableRegistry::CandidSerializeTypeTableRegistry() = default;
CandidSerializeTypeTableRegistry::~CandidSerializeTypeTableRegistry() = default;

void CandidSerializeTypeTableRegistry::clear() {}

size_t CandidSerializeTypeTableRegistry::add_or_replace_type_table(
    size_t, const VecBytes &) {
  return static_cast<size_t>(-1);
}

void CandidSerializeTypeTableRegistry::prune() {}

const VecBytes &
CandidSerializeTypeTableRegistry::get_type_table_vec_bytes(size_t) const {
  static const VecBytes empty;
  return empty;
}
