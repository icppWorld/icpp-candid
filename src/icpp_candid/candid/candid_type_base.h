// The base class for all Candid Types

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "candid_opcode.h"
#include "icpp_hooks.h"
#include "vec_bytes.h"

class CandidTypeBase {
public:
  CandidTypeBase();
  ~CandidTypeBase();

  virtual void trap_if_wrong_type_on_wire(const std::string &type_on_wire);
  int get_datatype_opcode() { return m_datatype_opcode; }
  uint8_t get_datatype_hex() { return m_datatype_hex; }
  std::string get_datatype_textual() { return m_datatype_textual; }
  int get_content_type_opcode() { return m_datatype_opcode; }
  uint8_t get_content_type_hex() { return m_datatype_hex; }
  std::string get_content_type_textual() { return m_datatype_textual; }
  VecBytes get_T() { return m_T; }
  __uint128_t get_type_table_index() { return m_type_table_index; }
  VecBytes get_I() { return m_I; }
  VecBytes get_M() { return m_M; }
  uint64_t get_v_size() { return m_v_size; }

  std::vector<uint32_t> get_field_ids() { return m_field_ids; }
  std::vector<std::string> get_field_names() { return m_field_names; }
  std::vector<int> get_field_datatypes() { return m_field_datatypes; }
  std::vector<std::shared_ptr<CandidTypeBase>> get_field_ptrs() {
    return m_fields_ptrs;
  }

  uint32_t idl_hash(const std::string &s);

  // Provide a less than operator to enable sorting
  friend bool operator<(CandidTypeBase const &lhs, CandidTypeBase const &rhs) {
    return lhs.m_datatype_opcode < rhs.m_datatype_opcode;
  }

  // Virtual methods to be implemented by the <comptype> CandidTypes
  // Non <comptype> should not call this method.
  virtual bool decode_T(const VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
  virtual void set_content_type();

  // Virtual methods to be implemented by all CandidTypes
  virtual bool decode_M(VecBytes B, __uint128_t &offset,
                        std::string &parse_error);
  virtual void encode_M();

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
  std::vector<std::shared_ptr<CandidTypeBase>> m_fields_ptrs;

  // The encoded byte vector for the Type Table
  VecBytes m_T;

  // The unique type table index into the registry,
  // As maintained by the CandidSerializeTypeTableRegistry singleton
  __uint128_t m_type_table_index{0};

  // The encoded byte vector for the Candid Type
  VecBytes m_I;

  // The encoded byte vector for the value
  VecBytes m_M;
};