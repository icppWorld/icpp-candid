// The base class for all Candid Types

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <limits>

#include "candid_constants.h"
#include "icpp_hooks.h"
#include "candid_type_graph.h"
#include "candid_value.h"
#include "vec_bytes.h"

// non-templated base class
class CandidTypeRoot {
public:
  CandidTypeRoot();
  ~CandidTypeRoot();

  // virtual methods that all derived classes MUST implement
  virtual CandidType toCandidType();

  // Virtual methods to be implemented by the <comptype> CandidTypes
  // Non <comptype> should not call this method.
  virtual void set_content_type();

  // Virtual methods to be implemented by VecRecord
  virtual void create_dummy_record_encode();

  virtual void encode_M();

  std::optional<bool> get_is_internal() { return m_is_internal; }
  void set_is_internal(bool is_internal) { m_is_internal = is_internal; }

  int get_datatype_opcode() { return m_datatype_opcode; }
  uint8_t get_datatype_hex() { return m_datatype_hex; }
  std::string get_datatype_textual() { return m_datatype_textual; }

  // Vec & Opt
  int get_content_opcode() { return m_content_opcode; }
  uint8_t get_content_hex() { return m_content_hex; }
  std::string get_content_textual() { return m_content_textual; }
  // OptRecord, ...Opt<constype>
  int get_content_datatype() { return m_content_datatype; }
  void set_content_datatype(int content_datatype) {
    m_content_datatype = content_datatype;
  }

  // Deprecated, since 7.1.0rc2 builds the type table at serialize time:
  // get_T() is empty, get_type_table_index() is size_t(-1)
  VecBytes get_T() { return VecBytes(); }
  __uint128_t get_type_table_index() { return static_cast<size_t>(-1); }
  VecBytes get_I() { return m_I; }
  VecBytes get_M() { return m_M; }
  // False if get_M() is not this object's value: a CandidTypeValue that only
  // gives a type or receives a decoded value, or anything that holds one
  bool has_M() const { return m_has_M; }
  uint64_t get_v_size() { return m_v_size; }

  // Record: The field data
  std::vector<uint32_t> get_field_ids() { return m_field_ids; }
  std::vector<std::string> get_field_names() { return m_field_names; }
  std::vector<int> get_field_datatypes() { return m_field_datatypes; }
  std::vector<int> get_field_opcodes() { return m_field_opcodes; }
  std::vector<int> get_field_content_datatypes() {
    return m_field_content_datatypes;
  }
  std::vector<int> get_field_content_opcodes() {
    return m_field_content_opcodes;
  }
  std::vector<std::shared_ptr<CandidTypeRoot>> get_field_ptrs() {
    return m_field_ptrs;
  }

  // Decoding, with CandidDecoder:
  // The type a value of this type is decoded at, added to g: a primitive
  // opcode, or a node of g. The classes of a primitive type, and the vec and
  // opt of one, get it from their opcodes; the others override it.
  virtual int graph_type(CandidTypeGraph &g);
  // Fills the bound C++ variable(s) with a decoded value of graph_type()
  virtual void bind(const CandidValue &v);
  // For a vec: appends one decoded element (the columns of a
  // CandidTypeVecRecord)
  virtual void bind_push_back(const CandidValue &e);

  uint32_t idl_hash(const std::string &s);

  template <typename T, typename U> bool in_range(U value);

protected:
  // The datatype
  int m_datatype_opcode{CANDID_UNDEF};
  uint8_t m_datatype_hex{0x00};
  std::string m_datatype_textual{""};

  // The content type (only used by vector & opt)
  int m_content_datatype{CANDID_UNDEF};
  int m_content_opcode{CANDID_UNDEF};
  uint8_t m_content_hex{0x00};
  std::string m_content_textual{""};

  // The data vector size (only used by vector)
  uint64_t m_v_size{0};

  // The field data (only used by record & variant)
  std::vector<uint32_t> m_field_ids;      // id | hash
  std::vector<std::string> m_field_names; // field name (label)
  std::vector<int> m_field_datatypes;
  std::vector<int> m_field_opcodes;
  std::vector<int> m_field_content_datatypes;
  std::vector<int> m_field_content_opcodes;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_field_ptrs;

  // Sort the field data by field id (hash), as the Candid spec requires for
  // the fields of a record and of a variant in the type table
  void sort_fields_by_id();

  // The encoded byte vector for the Candid Type
  VecBytes m_I;

  // The encoded byte vector for the value
  VecBytes m_M;
  bool m_has_M{true};

  // flag set when the instance is created internally, to decode wire
  std::optional<bool> m_is_internal;
};

// Templated base class
template <typename Derived> class CandidTypeBase : public CandidTypeRoot {
public:
  CandidTypeBase();
  ~CandidTypeBase();

  // Returns a copy as a CandidType (a variant), to be used in std::visit
  // we can convert pointers of derived classes that inherit from CandidTypeBase
  // into their corresponding std::variant, CandidType. This enables us to
  // utilize std::visit on the methods of the actual derived class.
  CandidType toCandidType();
};

template <typename T, typename U> bool CandidTypeRoot::in_range(U value) {
  return value >= std::numeric_limits<T>::min() &&
         value <= std::numeric_limits<T>::max();
}