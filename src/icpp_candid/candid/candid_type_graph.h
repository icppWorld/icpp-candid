// A Candid type graph: the types of the Candid spec, recursion included.
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#type-structure
//
// A type reference is an int:
//   - a negative value is a primitive type, by its opcode (-1 null ... -17
//     empty, -24 principal), exactly as on the wire;
//   - a value >= 0 is the index of a node in the graph: opt, vec, record,
//     variant, func, service, or a future type (wire only).
//
// Nodes refer to each other by index, so recursive types are natural:
//   type List = opt record { head : int; tail : List }
// is a node `opt -> 1` and a node `record { head : int; tail : 0 }`.
//
// The same structure holds the types read from a message's type table and the
// types a canister expects, so one subtype check and one decoder serve both.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum class CandidKind : uint8_t {
  Null,
  Bool,
  Nat,
  Int,
  Nat8,
  Nat16,
  Nat32,
  Nat64,
  Int8,
  Int16,
  Int32,
  Int64,
  Float32,
  Float64,
  Text,
  Reserved,
  Empty,
  Principal,
  Opt,
  Vec,
  Record,
  Variant,
  Func,
  Service,
  Future,
};

// Function annotations, as on the wire
constexpr uint8_t CANDID_FUNC_QUERY = 1;
constexpr uint8_t CANDID_FUNC_ONEWAY = 2;
constexpr uint8_t CANDID_FUNC_COMPOSITE_QUERY = 3;

struct CandidGraphField {
  uint32_t id{0};
  int type{-1};
};

struct CandidGraphMethod {
  std::string name;
  int type{-1}; // must refer to a func node
};

struct CandidGraphNode {
  CandidKind kind{CandidKind::Empty};
  int inner{-1};                          // opt, vec
  std::vector<CandidGraphField> fields;   // record, variant: sorted by id
  std::vector<int> args;                  // func
  std::vector<int> rets;                  // func
  std::vector<uint8_t> modes;             // func: annotations, sorted
  std::vector<CandidGraphMethod> methods; // service: sorted by name
  bool defined{true};                     // false while reserved
};

class CandidTypeGraph {
public:
  // --- Type references ---
  static bool is_node(int t) { return t >= 0; }
  // The opcode of a primitive kind, or 0 if the kind is not primitive
  static int prim(CandidKind k);
  // The primitive kind of an opcode; false if it is not a primitive opcode
  static bool prim_kind(int opcode, CandidKind &k);
  static bool is_fixed_width(CandidKind k, size_t &width);

  CandidKind kind(int t) const;
  const CandidGraphNode &node(int t) const { return m_nodes[size_t(t)]; }
  size_t size() const { return m_nodes.size(); }
  bool is_valid_ref(int t) const;

  // --- Building ---
  // A placeholder node, defined later with define(), for recursive types
  int reserve();
  void define(int idx, CandidGraphNode n);
  int add(CandidGraphNode n);

  int opt(int inner);
  int vec(int inner);
  int record(std::vector<CandidGraphField> fields);  // sorted here
  int variant(std::vector<CandidGraphField> fields); // sorted here
  int func(std::vector<int> args, std::vector<int> rets,
           std::vector<uint8_t> modes);
  int service(std::vector<CandidGraphMethod> methods); // sorted here

  // Checks a graph built with the builder: every reserved node defined, every
  // reference in range, fields unique, methods unique and of func type. A
  // graph read from the wire is checked as it is read.
  bool validate(std::string &err) const;

  // --- Queries the spec's rules need ---
  // null <: t, as the record and opt rules use it: t is null, opt or reserved
  bool is_null_like(int t) const;
  // An option whose constituent is again an infinite option:
  // `type Opt = opt Opt`, or `type A = opt B; type B = opt A`
  bool is_infinite_opt(int t) const;
  // Values of this type never take bytes on the wire (null, reserved, and
  // records of such types)
  bool is_zero_width(int t) const;

  // The field hash of a Candid field or method name
  static uint32_t idl_hash(const std::string &name);

  // Types the decoder treats as empty: records that can only contain
  // themselves, which have no finite values (as dfinity/candid does)
  void replace_empty_records();

  // A readable form, for error messages
  std::string to_string(int t) const;

private:
  std::vector<CandidGraphNode> m_nodes;
  std::vector<int8_t> m_zero_width; // cache: -1 unknown, 0 no, 1 yes

  void to_string_(int t, std::string &out, std::vector<bool> &seen) const;
};
