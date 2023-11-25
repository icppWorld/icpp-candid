// The base class for all Candid Types

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "candid_constants.h"
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
  virtual void set_content_type();

  // Virtual methods to be implemented by VecRecord
  virtual void create_dummy_record_encode();

  // Virtual methods to be implemented by all CandidTypes
  virtual bool decode_M(CandidDeserialize &de, VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
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

  std::vector<uint32_t> get_field_ids_wire() { return m_field_ids_wire; }
  std::vector<int> get_field_datatypes_wire() { return m_field_datatypes_wire; }
  std::vector<int> get_field_opcodes_wire() { return m_field_opcodes_wire; }
  std::vector<int> get_field_content_datatypes_wire() {
    return m_field_content_datatypes_wire;
  }
  std::vector<int> get_field_content_opcodes_wire() {
    return m_field_content_opcodes_wire;
  }
  std::vector<std::shared_ptr<CandidTypeRoot>> get_field_ptrs_wire() {
    return m_field_ptrs_wire;
  }

  virtual void set_fields_wire(std::shared_ptr<CandidTypeRoot> p_from_wire);

  uint32_t idl_hash(const std::string &s);

  // Methods used by instances created during deserialization of bytes from wire
  void trap_if_wrong_type_on_wire(const std::string &type_on_wire);
  virtual bool decode_T(const VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
  virtual void finish_decode_T(CandidDeserialize &de);

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

  // To help with decoding checks in decode_M
  // The deserialized fields (of a record) found on wire (decode_T)
  std::vector<uint32_t> m_field_ids_wire;
  std::vector<int> m_field_datatypes_wire;
  std::vector<int> m_field_opcodes_wire;
  std::vector<int> m_field_content_datatypes_wire;
  std::vector<int> m_field_content_opcodes_wire;
  std::vector<std::shared_ptr<CandidTypeRoot>> m_field_ptrs_wire;

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
