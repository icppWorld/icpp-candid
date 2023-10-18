// The base class for all Candid Types

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "icpp_hooks.h"
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
  virtual bool decode_T(const VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
  virtual void set_content_type();

  // Virtual methods to be implemented by all CandidTypes
  virtual bool decode_M(VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
  virtual void encode_M();

  void trap_if_wrong_type_on_wire(const std::string &type_on_wire);
  int get_datatype_opcode() { return m_datatype_opcode; }
  uint8_t get_datatype_hex() { return m_datatype_hex; }
  std::string get_datatype_textual() { return m_datatype_textual; }
  int get_content_type_opcode() { return m_content_type_opcode; }
  uint8_t get_content_type_hex() { return m_content_type_hex; }
  std::string get_content_type_textual() { return m_content_type_textual; }
  VecBytes get_T() { return m_T; }
  __uint128_t get_type_table_index() { return m_type_table_index; }
  VecBytes get_I() { return m_I; }
  VecBytes get_M() { return m_M; }
  uint64_t get_v_size() { return m_v_size; }

  // Virtual method to be implemented by all CandidTypeVecXXX to push_back a value into the internal std::vector<T>
  virtual void push_back_value(CandidTypeRoot &value);

  // Record: The field data
  std::vector<uint32_t> get_field_ids() { return m_field_ids; }
  std::vector<std::string> get_field_names() { return m_field_names; }
  std::vector<int> get_field_datatypes() { return m_field_datatypes; }
  std::vector<std::shared_ptr<CandidTypeRoot>> get_field_ptrs() {
    return m_fields_ptrs;
  }
  uint32_t idl_hash(const std::string &s);

protected:
  // The datatype
  int m_datatype_opcode{0};
  uint8_t m_datatype_hex{0x00};
  std::string m_datatype_textual{""};

  // The content type (only used by vector & opt)
  int m_content_type_opcode{0};
  uint8_t m_content_type_hex{0x00};
  std::string m_content_type_textual{""};

  // The data vector size (only used by vector)
  uint64_t m_v_size{0};

  // The field data (only used by record)
  std::vector<uint32_t> m_field_ids; // id | hash
  std::vector<std::string> m_field_names;
  std::vector<int> m_field_datatypes;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_fields_ptrs;

  // The encoded byte vector for the Type Table
  VecBytes m_T;

  // The unique type table index into the registry,
  // As maintained by the CandidSerializeTypeTableRegistry singleton
  // Note: setting it to -1 is ok, because it relies on defined unsigned
  //       integer underflow behavior. It will be set to to maximum
  //       representable value of size_t.
  size_t m_type_table_index{static_cast<size_t>(-1)};

  // The encoded byte vector for the Candid Type
  VecBytes m_I;

  // The encoded byte vector for the value
  VecBytes m_M;
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
