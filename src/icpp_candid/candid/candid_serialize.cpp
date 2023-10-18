// Candid serialization class
// https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results

#include <variant>

#include "candid_assert.h"
#include "candid_serialize.h"
#include "candid_serialize_type_table_registry.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"

// Default constructor handles nullary input '()'
CandidSerialize::CandidSerialize() { serialize(); }
CandidSerialize::CandidSerialize(const CandidType &a) {
  m_A.append(a);
  serialize();
}
CandidSerialize::CandidSerialize(const CandidArgs &A) : m_A{A} { serialize(); }

CandidSerialize::~CandidSerialize() {}

void CandidSerialize::serialize() {
  // https://github.com/dfinity/candid/blob/master/spec/Candid.md#parameters-and-results
  // B(kv* : <datatype>*) =
  // i8('D') i8('I') i8('D') i8('L')      magic number
  // T*(<comptype>*)                      type definition table
  // I*(<datatype>*)                      types of the argument list
  // M(kv* : <datatype>*)                 values of argument list

  m_B.clear();
  CandidSerializeTypeTableRegistry::get_instance().prune();
  std::vector<CandidSerializeTypeTableEntry> unique_type_tables =
      CandidSerializeTypeTableRegistry::get_instance().get_unique_type_tables();

  // -------------------------------------------------------------------------------------
  // i8('D') i8('I') i8('D') i8('L')      magic number
  m_B.append_didl();

  // -------------------------------------------------------------------------------------
  // T*(<comptype>*)                      type definition table
  // Write the unique TypeTables of the comptypes:
  // (-) Constructed Types (opt, vec, record, variant)
  // (-) Reference Types (func, service)

  // Append the number of unique type tables
  m_B.append_uleb128(__uint128_t(unique_type_tables.size()));

  // Append the type tables
  for (const CandidSerializeTypeTableEntry &entry : unique_type_tables) {
    const VecBytes &T = entry.type_table_vec_bytes;
    for (std::byte b : T.vec()) {
      m_B.append_byte(b);
    }
  }

  // -------------------------------------------------------------------------------------
  // I*(<datatype>*)                      types of the argument list
  m_B.append_uleb128(__uint128_t(m_A.m_args_ptrs.size()));

  for (size_t i = 0; i < m_A.m_args_ptrs.size(); ++i) {
    VecBytes T = m_A.m_args_ptrs[i]->get_T();
    if (T.size() > 0) {
      // For <comptypes>, we use the index into the type table we defined above
      __uint128_t type_table_index = m_A.m_args_ptrs[i]->get_type_table_index();
      m_B.append_uleb128(type_table_index);
    } else {
      // For <primtypes>, use the Opcode, already stored
      VecBytes I = m_A.m_args_ptrs[i]->get_I();
      for (std::byte b : I.vec()) {
        m_B.append_byte(b);
      }
    }
  }

  // -------------------------------------------------------------------------------------
  // M(kv* : <datatype>*)                 values of argument list
  for (std::shared_ptr<CandidTypeRoot> p_c : m_A.m_args_ptrs) {
    VecBytes M = p_c->get_M();
    for (std::byte b : M.vec()) {
      m_B.append_byte(b);
    }
  }

  // Append R
  // Was never implemented in Candid, although it is still in the spec
  // https://www.joachim-breitner.de/blog/786-A_Candid_explainer__Quirks
}

// Assert the serialized candid VecBytes against a string in "hex" format (didc encode)
int CandidSerialize::assert_candid(const std::string &candid_expected,
                                   const bool &assert_value) {
  return CandidAssert::assert_candid(m_B, candid_expected, assert_value);
}
