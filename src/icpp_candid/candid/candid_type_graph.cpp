// A Candid type graph: the types of the Candid spec, recursion included.

#include "candid_type_graph.h"

#include <algorithm>
#include <functional>
#include <utility>

int CandidTypeGraph::prim(CandidKind k) {
  switch (k) {
  case CandidKind::Null:
    return -1;
  case CandidKind::Bool:
    return -2;
  case CandidKind::Nat:
    return -3;
  case CandidKind::Int:
    return -4;
  case CandidKind::Nat8:
    return -5;
  case CandidKind::Nat16:
    return -6;
  case CandidKind::Nat32:
    return -7;
  case CandidKind::Nat64:
    return -8;
  case CandidKind::Int8:
    return -9;
  case CandidKind::Int16:
    return -10;
  case CandidKind::Int32:
    return -11;
  case CandidKind::Int64:
    return -12;
  case CandidKind::Float32:
    return -13;
  case CandidKind::Float64:
    return -14;
  case CandidKind::Text:
    return -15;
  case CandidKind::Reserved:
    return -16;
  case CandidKind::Empty:
    return -17;
  case CandidKind::Principal:
    return -24;
  default:
    return 0;
  }
}

bool CandidTypeGraph::prim_kind(int opcode, CandidKind &k) {
  if (opcode == -24) {
    k = CandidKind::Principal;
    return true;
  }
  if (opcode >= -17 && opcode <= -1) {
    k = static_cast<CandidKind>(-opcode - 1);
    return true;
  }
  return false;
}

bool CandidTypeGraph::is_fixed_width(CandidKind k, size_t &width) {
  switch (k) {
  case CandidKind::Bool:
  case CandidKind::Nat8:
  case CandidKind::Int8:
    width = 1;
    return true;
  case CandidKind::Nat16:
  case CandidKind::Int16:
    width = 2;
    return true;
  case CandidKind::Nat32:
  case CandidKind::Int32:
  case CandidKind::Float32:
    width = 4;
    return true;
  case CandidKind::Nat64:
  case CandidKind::Int64:
  case CandidKind::Float64:
    width = 8;
    return true;
  default:
    return false;
  }
}

CandidKind CandidTypeGraph::kind(int t) const {
  if (t >= 0) return m_nodes[size_t(t)].kind;
  CandidKind k = CandidKind::Empty;
  prim_kind(t, k);
  return k;
}

bool CandidTypeGraph::is_valid_ref(int t) const {
  if (t >= 0) return size_t(t) < m_nodes.size();
  CandidKind k;
  return prim_kind(t, k);
}

int CandidTypeGraph::reserve() {
  CandidGraphNode n;
  n.defined = false;
  m_nodes.push_back(n);
  m_zero_width.clear();
  m_infinite_opt.clear();
  return int(m_nodes.size() - 1);
}

void CandidTypeGraph::define(int idx, CandidGraphNode n) {
  n.defined = true;
  m_nodes[size_t(idx)] = std::move(n);
  m_zero_width.clear();
  m_infinite_opt.clear();
}

int CandidTypeGraph::add(CandidGraphNode n) {
  n.defined = true;
  m_nodes.push_back(std::move(n));
  m_zero_width.clear();
  m_infinite_opt.clear();
  return int(m_nodes.size() - 1);
}

int CandidTypeGraph::opt(int inner) {
  CandidGraphNode n;
  n.kind = CandidKind::Opt;
  n.inner = inner;
  return add(std::move(n));
}

int CandidTypeGraph::vec(int inner) {
  CandidGraphNode n;
  n.kind = CandidKind::Vec;
  n.inner = inner;
  return add(std::move(n));
}

static void sort_fields(std::vector<CandidGraphField> &fields) {
  std::stable_sort(fields.begin(), fields.end(),
                   [](const CandidGraphField &a, const CandidGraphField &b) {
                     return a.id < b.id;
                   });
}

int CandidTypeGraph::record(std::vector<CandidGraphField> fields) {
  sort_fields(fields);
  CandidGraphNode n;
  n.kind = CandidKind::Record;
  n.fields = std::move(fields);
  return add(std::move(n));
}

int CandidTypeGraph::variant(std::vector<CandidGraphField> fields) {
  sort_fields(fields);
  CandidGraphNode n;
  n.kind = CandidKind::Variant;
  n.fields = std::move(fields);
  return add(std::move(n));
}

int CandidTypeGraph::func(std::vector<int> args, std::vector<int> rets,
                          std::vector<uint8_t> modes) {
  std::sort(modes.begin(), modes.end());
  CandidGraphNode n;
  n.kind = CandidKind::Func;
  n.args = std::move(args);
  n.rets = std::move(rets);
  n.modes = std::move(modes);
  return add(std::move(n));
}

int CandidTypeGraph::service(std::vector<CandidGraphMethod> methods) {
  std::stable_sort(methods.begin(), methods.end(),
                   [](const CandidGraphMethod &a, const CandidGraphMethod &b) {
                     return a.name < b.name;
                   });
  CandidGraphNode n;
  n.kind = CandidKind::Service;
  n.methods = std::move(methods);
  return add(std::move(n));
}

bool CandidTypeGraph::validate(std::string &err) const {
  auto ref_ok = [&](int t, const char *what) {
    if (!is_valid_ref(t)) {
      err = std::string("Type reference out of range in ") + what;
      return false;
    }
    return true;
  };
  for (size_t i = 0; i < m_nodes.size(); ++i) {
    const CandidGraphNode &n = m_nodes[i];
    if (!n.defined) {
      err = "Type #" + std::to_string(i) + " is reserved but never defined";
      return false;
    }
    switch (n.kind) {
    case CandidKind::Opt:
    case CandidKind::Vec:
      if (!ref_ok(n.inner, "opt/vec")) return false;
      break;
    case CandidKind::Record:
    case CandidKind::Variant:
      for (size_t f = 0; f < n.fields.size(); ++f) {
        if (!ref_ok(n.fields[f].type, "record/variant")) return false;
        // Sorted by id, as the decoder and CandidValue::field look fields
        // up; define() and add() take them as given
        if (f > 0 && n.fields[f - 1].id == n.fields[f].id) {
          err = "Duplicate field id " + std::to_string(n.fields[f].id);
          return false;
        }
        if (f > 0 && n.fields[f - 1].id > n.fields[f].id) {
          err = "Field ids are not sorted at " + std::to_string(n.fields[f].id);
          return false;
        }
      }
      break;
    case CandidKind::Func:
      for (int t : n.args)
        if (!ref_ok(t, "func")) return false;
      for (int t : n.rets)
        if (!ref_ok(t, "func")) return false;
      break;
    case CandidKind::Service:
      for (size_t m = 0; m < n.methods.size(); ++m) {
        int t = n.methods[m].type;
        if (!ref_ok(t, "service")) return false;
        if (t < 0 || m_nodes[size_t(t)].kind != CandidKind::Func) {
          err = "Method " + n.methods[m].name + " is not of func type";
          return false;
        }
        if (m > 0 && n.methods[m - 1].name == n.methods[m].name) {
          err = "Duplicate method " + n.methods[m].name;
          return false;
        }
        if (m > 0 && n.methods[m - 1].name > n.methods[m].name) {
          err = "Method names are not sorted at " + n.methods[m].name;
          return false;
        }
      }
      break;
    default:
      break;
    }
  }
  return true;
}

bool CandidTypeGraph::is_null_like(int t) const {
  CandidKind k = kind(t);
  return k == CandidKind::Null || k == CandidKind::Opt ||
         k == CandidKind::Reserved;
}

bool CandidTypeGraph::is_infinite_opt(int t) const {
  // Infinite: the chain of opt constituents comes back to a node already on
  // it. Worked out for every node at once, in one pass over the chains.
  auto &cache = const_cast<std::vector<int8_t> &>(m_infinite_opt);
  if (cache.size() != m_nodes.size()) {
    // 0 unvisited, 1 on the chain being followed, 2 finite, 3 infinite
    std::vector<uint8_t> state(m_nodes.size(), 0);
    std::vector<int> chain;
    for (size_t i = 0; i < m_nodes.size(); ++i) {
      int n = int(i);
      chain.clear();
      while (n >= 0 && state[size_t(n)] == 0 &&
             m_nodes[size_t(n)].kind == CandidKind::Opt) {
        state[size_t(n)] = 1;
        chain.push_back(n);
        n = m_nodes[size_t(n)].inner;
      }
      // Where the chain ends decides it: back on itself, or at an infinite
      // one, it is infinite
      uint8_t result =
          n >= 0 && (state[size_t(n)] == 1 || state[size_t(n)] == 3) ? 3 : 2;
      for (int c : chain)
        state[size_t(c)] = result;
    }
    cache.assign(m_nodes.size(), 0);
    for (size_t i = 0; i < m_nodes.size(); ++i)
      cache[i] = state[i] == 3 ? 1 : 0;
  }
  return t >= 0 && cache[size_t(t)] == 1;
}

bool CandidTypeGraph::is_zero_width(int t) const {
  if (t < 0) {
    CandidKind k = kind(t);
    return k == CandidKind::Null || k == CandidKind::Reserved;
  }
  auto &cache = const_cast<std::vector<int8_t> &>(m_zero_width);
  if (cache.size() != m_nodes.size()) cache.assign(m_nodes.size(), -1);
  if (cache[size_t(t)] >= 0) return cache[size_t(t)] == 1;

  // A record all of whose fields are zero width. Without recursion, so the
  // stack it takes does not depend on the length of a chain of records in the
  // type table. A record on the path
  // being followed counts as not zero width: a record that reaches itself is
  // not zero width here (on the wire, such records are already empty).
  auto known = [&](int n, bool &z) {
    if (n < 0) {
      CandidKind k = kind(n);
      z = k == CandidKind::Null || k == CandidKind::Reserved;
      return true;
    }
    if (cache[size_t(n)] < 0) return false;
    z = cache[size_t(n)] == 1;
    return true;
  };
  std::vector<std::pair<int, size_t>> path{{t, 0}}; // node, next field
  cache[size_t(t)] = 0;
  while (!path.empty()) {
    int n = path.back().first;
    size_t k = path.back().second;
    const CandidGraphNode &node = m_nodes[size_t(n)];
    bool done = true, zero = node.kind == CandidKind::Record;
    while (zero && k < node.fields.size()) {
      int f = node.fields[k].type;
      bool fz;
      if (!known(f, fz)) {
        path.back().second = k;
        cache[size_t(f)] = 0;
        path.push_back({f, 0});
        done = false;
        break;
      }
      if (!fz) zero = false;
      ++k;
    }
    if (!done) continue;
    cache[size_t(n)] = zero ? 1 : 0;
    path.pop_back();
  }
  return cache[size_t(t)] == 1;
}

uint32_t CandidTypeGraph::idl_hash(const std::string &name) {
  uint32_t h = 0;
  for (unsigned char c : name)
    h = h * 223 + c;
  return h;
}

void CandidTypeGraph::replace_empty_records() {
  // As dfinity/candid's TypeEnv::replace_empty: a record is empty when one of
  // its fields leads, through records only, back to a record still being
  // visited. 0 = unvisited, 1 = in progress, 2 = not empty, 3 = empty.
  // Without recursion, so the stack it takes does not depend on the length of
  // a chain of records in the type table.
  std::vector<uint8_t> state(m_nodes.size(), 0);
  std::vector<std::pair<int, size_t>> path; // node, next field
  for (size_t i = 0; i < m_nodes.size(); ++i) {
    if (state[i] != 0) continue;
    state[i] = 1;
    path.push_back({int(i), 0});
    while (!path.empty()) {
      int n = path.back().first;
      size_t k = path.back().second;
      const CandidGraphNode &node = m_nodes[size_t(n)];
      bool done = true, empty = false;
      if (node.kind == CandidKind::Record) {
        for (; k < node.fields.size(); ++k) {
          int f = node.fields[k].type;
          if (f < 0) continue;
          uint8_t &s = state[size_t(f)];
          if (s == 0) {
            path.back().second = k; // back here when f is done
            s = 1;
            path.push_back({f, 0});
            done = false;
            break;
          }
          if (s == 1) s = 3; // a cycle
          if (s == 3) {
            empty = true;
            break;
          }
        }
      }
      if (!done) continue;
      // A cycle may already have marked this node empty
      if (state[size_t(n)] == 3) empty = true;
      state[size_t(n)] = empty ? 3 : 2;
      path.pop_back();
    }
  }
  for (size_t i = 0; i < m_nodes.size(); ++i) {
    if (state[i] == 3) {
      CandidGraphNode n;
      n.kind = CandidKind::Empty;
      m_nodes[i] = n;
    }
  }
  m_zero_width.clear();
  m_infinite_opt.clear();
}

static const char *kind_name(CandidKind k) {
  switch (k) {
  case CandidKind::Null:
    return "null";
  case CandidKind::Bool:
    return "bool";
  case CandidKind::Nat:
    return "nat";
  case CandidKind::Int:
    return "int";
  case CandidKind::Nat8:
    return "nat8";
  case CandidKind::Nat16:
    return "nat16";
  case CandidKind::Nat32:
    return "nat32";
  case CandidKind::Nat64:
    return "nat64";
  case CandidKind::Int8:
    return "int8";
  case CandidKind::Int16:
    return "int16";
  case CandidKind::Int32:
    return "int32";
  case CandidKind::Int64:
    return "int64";
  case CandidKind::Float32:
    return "float32";
  case CandidKind::Float64:
    return "float64";
  case CandidKind::Text:
    return "text";
  case CandidKind::Reserved:
    return "reserved";
  case CandidKind::Empty:
    return "empty";
  case CandidKind::Principal:
    return "principal";
  case CandidKind::Opt:
    return "opt";
  case CandidKind::Vec:
    return "vec";
  case CandidKind::Record:
    return "record";
  case CandidKind::Variant:
    return "variant";
  case CandidKind::Func:
    return "func";
  case CandidKind::Service:
    return "service";
  case CandidKind::Future:
    return "future";
  }
  return "?";
}

std::string CandidTypeGraph::to_string(int t) const {
  std::string out;
  std::vector<bool> seen(m_nodes.size(), false);
  to_string_(t, out, seen);
  return out;
}

void CandidTypeGraph::to_string_(int t, std::string &out,
                                 std::vector<bool> &seen) const {
  if (out.size() > 200) {
    out += "...";
    return;
  }
  if (t < 0 || size_t(t) >= m_nodes.size()) {
    out += kind_name(kind(t));
    return;
  }
  if (seen[size_t(t)]) {
    out += "#" + std::to_string(t);
    return;
  }
  seen[size_t(t)] = true;
  const CandidGraphNode &n = m_nodes[size_t(t)];
  out += kind_name(n.kind);
  auto tuple = [&](const std::vector<int> &ts) {
    out += "(";
    for (size_t i = 0; i < ts.size(); ++i) {
      if (i > 0) out += ", ";
      to_string_(ts[i], out, seen);
    }
    out += ")";
  };
  switch (n.kind) {
  case CandidKind::Opt:
  case CandidKind::Vec:
    out += " ";
    to_string_(n.inner, out, seen);
    break;
  case CandidKind::Record:
  case CandidKind::Variant:
    out += " {";
    for (const auto &f : n.fields) {
      out += " " + std::to_string(f.id) + ":";
      to_string_(f.type, out, seen);
      out += ";";
    }
    out += " }";
    break;
  case CandidKind::Func:
    out += " ";
    tuple(n.args);
    out += " -> ";
    tuple(n.rets);
    break;
  case CandidKind::Service:
    out += " {";
    for (const auto &m : n.methods) {
      out += " " + m.name + ":";
      to_string_(m.type, out, seen);
      out += ";";
    }
    out += " }";
    break;
  default:
    break;
  }
  seen[size_t(t)] = false;
}
