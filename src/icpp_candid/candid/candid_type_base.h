// The base class for all Candid Types

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <limits>

#include "candid_constants.h"
#include "icpp_hooks.h"
#include "candid_type_graph.h"
#include "candid_value.h"
#include "vec_bytes.h"

// non-templated base class
class CandidTypeRoot {
  // Binds elements that are thrown away, without encoding them first
  friend class CandidTypeVecOf;

public:
  CandidTypeRoot();

  // virtual methods that all derived classes MUST implement
  virtual CandidType toCandidType();

  // Virtual methods to be implemented by the <comptype> CandidTypes
  // Non <comptype> should not call this method.
  virtual void set_content_type();

  // Virtual methods to be implemented by VecRecord
  virtual void create_dummy_record_encode();

  // Changes only m_M and m_has_M: get_M() of a const object may call it
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
  // Without a copy: a reference, or the bytes moved out of a temporary
  const VecBytes &get_I() const & {
    refresh_I();
    return m_I;
  }
  VecBytes get_I() && {
    refresh_I();
    return std::move(m_I);
  }
  const VecBytes &get_M() const & {
    refresh_M();
    return m_M;
  }
  VecBytes get_M() && {
    refresh_M();
    return std::move(m_M);
  }
  // False if get_M() is not this object's value: a CandidTypeValue that only
  // gives a type or receives a decoded value, or anything that holds one
  bool has_M() const {
    refresh_M();
    return m_has_M;
  }
  uint64_t get_v_size() { return m_v_size; }

  // Record: The field data
  std::vector<uint32_t> get_field_ids() { return m_field_ids; }
  std::vector<std::string> get_field_names() { return m_field_names; }
  // The (content) datatype and opcode of each field, from the fields
  std::vector<int> get_field_datatypes() {
    return field_values(&CandidTypeRoot::get_datatype_opcode);
  }
  std::vector<int> get_field_opcodes() { return get_field_datatypes(); }
  std::vector<int> get_field_content_datatypes() {
    return field_values(&CandidTypeRoot::get_content_datatype);
  }
  std::vector<int> get_field_content_opcodes() {
    return field_values(&CandidTypeRoot::get_content_opcode);
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

  // True if this and other are certainly the same type, found without
  // building their type graphs: objects of one primitive class, or records
  // with the same field ids whose fields are. False if that is not certain.
  // The base takes a primitive opcode for the type, as graph_type does: a
  // class with a primitive opcode that overrides graph_type must override
  // this too (CandidTypeValue does).
  virtual bool certainly_same_type(const CandidTypeRoot &other) const;

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
  std::vector<std::shared_ptr<CandidTypeRoot>> m_field_ptrs;

  // Inserts a field at its place by id, keeping the fields sorted, as the
  // Candid spec requires for a record and a variant. Traps on a duplicate id;
  // kind ("record", "variant") is for that message.
  void insert_field(uint32_t field_id, std::string field_name,
                    std::shared_ptr<CandidTypeRoot> p, const char *kind);

  // The encoded byte vector for the Candid Type: the type's opcode, made when
  // get_I() reads it
  mutable VecBytes m_I;

  // The encoded byte vector for the value. A class that sets m_M_stale
  // re-encodes it with encode_M() when it is read, once, instead of after
  // every change.
  mutable VecBytes m_M;
  mutable bool m_has_M{true};
  mutable bool m_M_stale{false};

  // flag set when the instance is created internally, to decode wire
  std::optional<bool> m_is_internal;

private:
  std::vector<int> field_values(int (CandidTypeRoot::*get)()) const {
    std::vector<int> v;
    v.reserve(m_field_ptrs.size());
    for (const auto &p : m_field_ptrs)
      v.push_back(((*p).*get)());
    return v;
  }
  void refresh_I() const {
    if (m_I.size() == 0) m_I.append_byte(std::byte(m_datatype_hex));
  }

protected:
  // Encodes m_M if it is stale. encode_M() changes only the mutable m_M and
  // m_has_M
  void refresh_M() const {
    if (!m_M_stale) return;
    m_M_stale = false;
    const_cast<CandidTypeRoot *>(this)->encode_M();
  }
};

// Templated base class
template <typename Derived> class CandidTypeBase : public CandidTypeRoot {
public:
  CandidTypeBase();

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