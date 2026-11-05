// The Candid encoder: serialisation of values at a type.

#include "candid_encoder.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <utility>

namespace {

void write_leb(std::vector<uint8_t> &out, uint64_t v) {
  do {
    uint8_t b = v & 0x7f;
    v >>= 7;
    out.push_back(v ? uint8_t(b | 0x80) : b);
  } while (v);
}

void write_sleb(std::vector<uint8_t> &out, int64_t v) {
  for (;;) {
    uint8_t b = v & 0x7f;
    v >>= 7; // arithmetic: the sign repeats
    bool done = (v == 0 && (b & 0x40) == 0) || (v == -1 && (b & 0x40) != 0);
    out.push_back(done ? b : uint8_t(b | 0x80));
    if (done) return;
  }
}

void write_bytes(std::vector<uint8_t> &out, const uint8_t *s, size_t n) {
  write_leb(out, n);
  out.insert(out.end(), s, s + n);
}

// The children of a node, in the order the type table visits them: the
// inner type, the fields by id, the arguments then the results, the methods
// by name
std::vector<int> children(const CandidGraphNode &n) {
  std::vector<int> c;
  switch (n.kind) {
  case CandidKind::Opt:
  case CandidKind::Vec:
    c.push_back(n.inner);
    break;
  case CandidKind::Record:
  case CandidKind::Variant:
    for (const auto &f : n.fields)
      c.push_back(f.type);
    break;
  case CandidKind::Func:
    c = n.args;
    c.insert(c.end(), n.rets.begin(), n.rets.end());
    break;
  case CandidKind::Service:
    for (const auto &m : n.methods)
      c.push_back(m.type);
    break;
  default:
    break;
  }
  return c;
}

// Builds the type table for the argument types
class TableBuilder {
public:
  TableBuilder(const CandidTypeGraph &g, const CandidDecodeLimits &limits)
      : m_g(g), m_limits(limits), m_named(g.size(), false), m_key(g.size(), -1),
        m_idx(g.size(), -1), m_walked(g.size(), false) {
    for (size_t i = 0; i < g.size(); ++i)
      m_named[i] = g.is_named(int(i));
  }

  bool build(const std::vector<int> &args, std::vector<uint8_t> &out,
             std::string &err) {
    for (int a : args)
      if (!m_g.is_valid_ref(a)) {
        err = "An argument type is not in the type graph";
        return false;
      }
    name_cycles(args);
    for (int n : m_reached)
      compute_keys(n);
    for (int a : args)
      if (!number(a, err)) return false;

    out.insert(out.end(), {'D', 'I', 'D', 'L'});
    write_leb(out, m_entries.size());
    for (const auto &e : m_entries)
      out.insert(out.end(), e.begin(), e.end());
    write_leb(out, args.size());
    for (int a : args)
      write_sleb(out, ref(a));
    return true;
  }

private:
  const CandidTypeGraph &m_g;
  const CandidDecodeLimits &m_limits;
  std::vector<bool> m_named;  // named by reserve(), or to break a cycle
  std::vector<int> m_key;     // the interned structure of each node
  std::vector<int> m_idx;     // the type table index of each node
  std::vector<bool> m_walked; // compute_keys has been to it
  std::map<std::vector<int64_t>, int> m_keys;
  std::map<int, int> m_by_key; // structure -> index, of unnamed nodes
  std::vector<std::vector<uint8_t>> m_entries;
  std::vector<int> m_reached; // the nodes reachable from the arguments

  // A cycle that passes through no named node has no name to stop at: the
  // node the cycle returns to is treated as named. The walk is depth first
  // in table order, so the choice is the same every time.
  void name_cycles(const std::vector<int> &args) {
    std::vector<uint8_t> state(m_g.size(), 0); // 0 new, 1 on stack, 2 done
    std::vector<size_t> pos(m_g.size(), 0);
    struct Frame {
      int node;
      std::vector<int> kids;
      size_t next;
    };
    std::vector<Frame> stack;
    std::vector<long> last_named; // per stack position
    for (int a : args) {
      if (!CandidTypeGraph::is_node(a) || state[size_t(a)]) continue;
      auto push = [&](int n) {
        state[size_t(n)] = 1;
        pos[size_t(n)] = stack.size();
        long below = last_named.empty() ? -1 : last_named.back();
        last_named.push_back(m_named[size_t(n)] ? long(stack.size()) : below);
        stack.push_back({n, children(m_g.node(n)), 0});
      };
      push(a);
      while (!stack.empty()) {
        Frame &f = stack.back();
        if (f.next == f.kids.size()) {
          state[size_t(f.node)] = 2;
          m_reached.push_back(f.node);
          stack.pop_back();
          last_named.pop_back();
          continue;
        }
        int c = f.kids[f.next++];
        if (!CandidTypeGraph::is_node(c)) continue;
        if (state[size_t(c)] == 0) {
          push(c);
        } else if (state[size_t(c)] == 1) {
          size_t p = pos[size_t(c)];
          if (last_named.back() < long(p)) {
            m_named[size_t(c)] = true;
            for (size_t i = p; i < last_named.size(); ++i)
              last_named[i] = std::max(last_named[i], long(p));
          }
        }
      }
    }
  }

  // The structure of t: its kind, ids, names and annotations, and its
  // children, a named child by identity. An unnamed child's structure comes
  // first; every cycle passes through a named node, so this has none.
  void compute_keys(int t) {
    if (!CandidTypeGraph::is_node(t) || m_key[size_t(t)] != -1) return;
    struct Frame {
      int node;
      std::vector<int> kids;
      size_t next;
    };
    std::vector<Frame> stack;
    auto push = [&](int n) {
      m_walked[size_t(n)] = true;
      stack.push_back({n, children(m_g.node(n)), 0});
    };
    push(t);
    while (!stack.empty()) {
      Frame &f = stack.back();
      if (f.next < f.kids.size()) {
        int c = f.kids[f.next++];
        if (CandidTypeGraph::is_node(c) && !m_named[size_t(c)] &&
            m_key[size_t(c)] == -1 && !m_walked[size_t(c)])
          push(c);
        continue;
      }
      m_key[size_t(f.node)] = intern(f.node);
      stack.pop_back();
    }
  }

  void push_child(std::vector<int64_t> &sig, int c) const {
    if (!CandidTypeGraph::is_node(c)) {
      sig.push_back(0);
      sig.push_back(c);
    } else if (m_named[size_t(c)]) {
      sig.push_back(1);
      sig.push_back(c);
    } else {
      sig.push_back(2);
      sig.push_back(m_key[size_t(c)]);
    }
  }

  int intern(int t) {
    const CandidGraphNode &n = m_g.node(t);
    std::vector<int64_t> sig{int64_t(n.kind)};
    switch (n.kind) {
    case CandidKind::Opt:
    case CandidKind::Vec:
      push_child(sig, n.inner);
      break;
    case CandidKind::Record:
    case CandidKind::Variant:
      sig.push_back(int64_t(n.fields.size()));
      for (const auto &f : n.fields) {
        sig.push_back(f.id);
        push_child(sig, f.type);
      }
      break;
    case CandidKind::Func:
      for (const auto *list : {&n.args, &n.rets}) {
        sig.push_back(int64_t(list->size()));
        for (int c : *list)
          push_child(sig, c);
      }
      sig.push_back(int64_t(n.modes.size()));
      sig.insert(sig.end(), n.modes.begin(), n.modes.end());
      break;
    case CandidKind::Service:
      sig.push_back(int64_t(n.methods.size()));
      for (const auto &m : n.methods) {
        sig.push_back(int64_t(m.name.size()));
        sig.insert(sig.end(), m.name.begin(), m.name.end());
        push_child(sig, m.type);
      }
      break;
    default: // a future type: never equal to another
      sig.push_back(t);
      break;
    }
    auto it = m_keys.emplace(std::move(sig), int(m_keys.size())).first;
    return it->second;
  }

  int ref(int t) const {
    return CandidTypeGraph::is_node(t) ? m_idx[size_t(t)] : t;
  }

  // Gives t its index, or the index of an equal type seen before. A new
  // entry is numbered before its children are visited, and written after.
  bool number(int t, std::string &err) {
    struct Frame {
      int node;
      size_t entry;
      std::vector<int> kids;
      size_t next;
    };
    std::vector<Frame> stack;
    auto visit = [&](int n) -> bool {
      if (!CandidTypeGraph::is_node(n) || m_idx[size_t(n)] != -1) return true;
      auto it = m_by_key.find(m_key[size_t(n)]);
      if (it != m_by_key.end()) {
        m_idx[size_t(n)] = it->second;
        return true;
      }
      if (m_g.kind(n) == CandidKind::Future) {
        err = "Cannot encode a type of a future version of Candid";
        return false;
      }
      if (m_entries.size() >= m_limits.max_type_table) {
        err = "The type table has more than " +
              std::to_string(m_limits.max_type_table) + " entries";
        return false;
      }
      int idx = int(m_entries.size());
      m_idx[size_t(n)] = idx;
      // A named type is only found by name, as a Rust Var is, but an
      // unnamed equal type seen before is reused for it (as above)
      if (!m_named[size_t(n)]) m_by_key[m_key[size_t(n)]] = idx;
      m_entries.emplace_back();
      stack.push_back({n, size_t(idx), children(m_g.node(n)), 0});
      return true;
    };
    if (!visit(t)) return false;
    while (!stack.empty()) {
      Frame &f = stack.back();
      if (f.next < f.kids.size()) {
        if (!visit(f.kids[f.next++])) return false;
        continue;
      }
      write_entry(m_g.node(f.node), m_entries[f.entry]);
      stack.pop_back();
    }
    return true;
  }

  void write_entry(const CandidGraphNode &n, std::vector<uint8_t> &e) const {
    switch (n.kind) {
    case CandidKind::Opt:
      write_sleb(e, -18);
      write_sleb(e, ref(n.inner));
      break;
    case CandidKind::Vec:
      write_sleb(e, -19);
      write_sleb(e, ref(n.inner));
      break;
    case CandidKind::Record:
    case CandidKind::Variant:
      write_sleb(e, n.kind == CandidKind::Record ? -20 : -21);
      write_leb(e, n.fields.size());
      for (const auto &f : n.fields) {
        write_leb(e, f.id);
        write_sleb(e, ref(f.type));
      }
      break;
    case CandidKind::Func:
      write_sleb(e, -22);
      for (const auto *list : {&n.args, &n.rets}) {
        write_leb(e, list->size());
        for (int c : *list)
          write_sleb(e, ref(c));
      }
      write_leb(e, n.modes.size());
      e.insert(e.end(), n.modes.begin(), n.modes.end());
      break;
    case CandidKind::Service:
      write_sleb(e, -23);
      write_leb(e, n.methods.size());
      for (const auto &m : n.methods) {
        write_bytes(e, reinterpret_cast<const uint8_t *>(m.name.data()),
                    m.name.size());
        write_sleb(e, ref(m.type));
      }
      break;
    default:
      break;
    }
  }
};

// Writes values, checking each against its type
class ValueWriter {
public:
  ValueWriter(const CandidTypeGraph &g, std::vector<uint8_t> &out,
              const CandidDecodeLimits &limits)
      : m_g(g), m_out(out), m_limits(limits) {}

  bool write(int t, const CandidValue &v) {
    if (m_depth >= m_limits.max_depth)
      return fail("Values nest more than " +
                  std::to_string(m_limits.max_depth) + " deep");
    ++m_depth;
    size_t start = m_out.size();
    bool ok = write_(t, v);
    --m_depth;
    if (ok && m_out.size() == start) return count_zero_width(1);
    return ok;
  }

  std::string m_err;

private:
  const CandidTypeGraph &m_g;
  std::vector<uint8_t> &m_out;
  const CandidDecodeLimits &m_limits;
  size_t m_depth{0};
  size_t m_zero_width{0};

  bool fail(const std::string &why) {
    if (m_err.empty()) m_err = why;
    return false;
  }

  bool count_zero_width(size_t n) {
    if (n > m_limits.max_zero_width_values - m_zero_width)
      return fail("More than " +
                  std::to_string(m_limits.max_zero_width_values) +
                  " values that take no bytes");
    m_zero_width += n;
    return true;
  }

  bool write_principal(const std::vector<uint8_t> &bytes) {
    if (bytes.size() > 29) return fail("A principal is at most 29 bytes");
    m_out.push_back(1);
    write_bytes(m_out, bytes.data(), bytes.size());
    return true;
  }

  bool write_text(const std::string &s, const char *what) {
    const auto *p = reinterpret_cast<const uint8_t *>(s.data());
    if (!CandidDecoder::is_valid_utf8(p, s.size()))
      return fail(std::string("Invalid UTF-8 in a ") + what);
    write_bytes(m_out, p, s.size());
    return true;
  }

  bool write_fixed(CandidKind k, const CandidValue &v) {
    uint64_t raw = 0;
    switch (k) {
    case CandidKind::Nat8:
    case CandidKind::Nat16:
    case CandidKind::Nat32:
    case CandidKind::Nat64: {
      int bits = k == CandidKind::Nat8    ? 8
                 : k == CandidKind::Nat16 ? 16
                 : k == CandidKind::Nat32 ? 32
                                          : 64;
      if (bits < 64 && v.u >> bits) return fail("The nat is out of range");
      raw = v.u;
      break;
    }
    case CandidKind::Int8:
    case CandidKind::Int16:
    case CandidKind::Int32:
    case CandidKind::Int64: {
      int bits = k == CandidKind::Int8    ? 8
                 : k == CandidKind::Int16 ? 16
                 : k == CandidKind::Int32 ? 32
                                          : 64;
      if (bits < 64) {
        int64_t lim = int64_t(1) << (bits - 1);
        if (v.i < -lim || v.i >= lim) return fail("The int is out of range");
      }
      raw = uint64_t(v.i);
      break;
    }
    case CandidKind::Float32: {
      float f = float(v.f64);
      uint32_t r32;
      std::memcpy(&r32, &f, 4);
      raw = r32;
      break;
    }
    case CandidKind::Float64:
      std::memcpy(&raw, &v.f64, 8);
      break;
    default:
      return fail("Internal error: not a fixed-width type");
    }
    size_t width = 0;
    CandidTypeGraph::is_fixed_width(k, width);
    for (size_t i = 0; i < width; ++i)
      m_out.push_back(uint8_t(raw >> (8 * i)));
    return true;
  }

  bool write_vec(int t, const CandidValue &v) {
    int inner = m_g.node(t).inner;
    CandidKind ik = m_g.kind(inner);
    if (v.packed) {
      size_t width = 0;
      if (!CandidTypeGraph::is_fixed_width(ik, width) || v.elem_kind != ik)
        return fail("The packed vec does not have the vec's element type");
      if (v.bytes.size() % width != 0)
        return fail("The packed vec's bytes are not whole elements");
      if (ik == CandidKind::Bool)
        for (uint8_t b : v.bytes)
          if (b > 1) return fail("A bool must be 0 or 1");
      write_leb(m_out, v.bytes.size() / width);
      m_out.insert(m_out.end(), v.bytes.begin(), v.bytes.end());
      return true;
    }
    if (v.repeat) {
      if (!m_g.is_zero_width(inner))
        return fail("Only a vec of values that take no bytes can repeat");
      if (v.items.size() != 1)
        return fail("A repeating vec holds its one element");
      write_leb(m_out, v.repeat);
      // The element takes no bytes: checked once, counted repeat times
      size_t before = m_zero_width;
      if (!write(inner, v.items[0])) return false;
      m_zero_width = before;
      return count_zero_width(v.repeat);
    }
    write_leb(m_out, v.items.size());
    for (const auto &item : v.items)
      if (!write(inner, item)) return false;
    return true;
  }

  bool write_record(int t, const CandidValue &v) {
    const auto &fields = m_g.node(t).fields;
    size_t k = 0;
    bool ok = true;
    v.for_each_field([&](uint32_t id, const CandidValue &fv) {
      if (!ok) return;
      if (k >= fields.size() || fields[k].id != id) {
        ok = fail("The record value has a field " + std::to_string(id) +
                  " that its type does not have, or not in this place");
        return;
      }
      int ft = fields[k++].type;
      if (fv.absent) {
        // An absent field: written as the value null coerces to
        if (!m_g.is_null_like(ft)) {
          ok = fail("Field " + std::to_string(id) +
                    " is absent, but its type is not null, opt or reserved");
          return;
        }
        if (m_g.kind(ft) == CandidKind::Opt) m_out.push_back(0);
        return;
      }
      ok = write(ft, fv);
    });
    if (!ok) return false;
    if (k != fields.size())
      return fail("The record value lacks field " +
                  std::to_string(fields[k].id));
    return true;
  }

  bool write_(int t, const CandidValue &v) {
    CandidKind k = m_g.kind(t);
    if (k == CandidKind::Reserved) return true; // any value, no bytes
    if (k == CandidKind::Empty) return fail("No value has the type empty");
    if (v.kind != k)
      return fail("A value of another type than the expected " +
                  m_g.to_string(t));
    switch (k) {
    case CandidKind::Null:
      return true;
    case CandidKind::Bool:
      m_out.push_back(v.b ? 1 : 0);
      return true;
    case CandidKind::Nat:
      if (!v.n.to_leb128(m_out)) return fail("A nat cannot be negative");
      return true;
    case CandidKind::Int:
      v.n.to_sleb128(m_out);
      return true;
    case CandidKind::Nat8:
    case CandidKind::Nat16:
    case CandidKind::Nat32:
    case CandidKind::Nat64:
    case CandidKind::Int8:
    case CandidKind::Int16:
    case CandidKind::Int32:
    case CandidKind::Int64:
    case CandidKind::Float32:
    case CandidKind::Float64:
      return write_fixed(k, v);
    case CandidKind::Text:
      return write_text(v.text, "text");
    case CandidKind::Principal:
    case CandidKind::Service:
      return write_principal(v.bytes);
    case CandidKind::Func:
      m_out.push_back(1);
      if (!write_principal(v.bytes)) return false;
      return write_text(v.text, "method name");
    case CandidKind::Opt:
      if (v.items.empty()) {
        m_out.push_back(0);
        return true;
      }
      if (v.items.size() != 1) return fail("An opt holds at most one value");
      m_out.push_back(1);
      return write(m_g.node(t).inner, v.items[0]);
    case CandidKind::Vec:
      return write_vec(t, v);
    case CandidKind::Record:
      return write_record(t, v);
    case CandidKind::Variant: {
      const auto &fields = m_g.node(t).fields;
      if (v.ids.size() != 1 || v.items.size() != 1)
        return fail("A variant value holds one case");
      auto it = std::lower_bound(
          fields.begin(), fields.end(), v.ids[0],
          [](const CandidGraphField &f, uint32_t id) { return f.id < id; });
      if (it == fields.end() || it->id != v.ids[0])
        return fail("The variant has no case " + std::to_string(v.ids[0]));
      write_leb(m_out, uint64_t(it - fields.begin()));
      return write(it->type, v.items[0]);
    }
    default:
      return fail("Cannot encode a value of type " + m_g.to_string(t));
    }
  }
};

} // namespace

bool CandidEncoder::encode_header(const CandidTypeGraph &g,
                                  const std::vector<int> &args,
                                  std::vector<uint8_t> &out, std::string &err,
                                  const CandidDecodeLimits &limits) {
  if (!g.validate(err)) return false;
  std::vector<uint8_t> header;
  if (!TableBuilder(g, limits).build(args, header, err)) return false;
  out.insert(out.end(), header.begin(), header.end());
  return true;
}

bool CandidEncoder::encode_value(const CandidTypeGraph &g, int t,
                                 const CandidValue &v,
                                 std::vector<uint8_t> &out, std::string &err,
                                 const CandidDecodeLimits &limits) {
  if (!g.is_valid_ref(t)) {
    err = "The type is not in the type graph";
    return false;
  }
  std::vector<uint8_t> bytes;
  ValueWriter w(g, bytes, limits);
  if (!w.write(t, v)) {
    err = w.m_err;
    return false;
  }
  out.insert(out.end(), bytes.begin(), bytes.end());
  return true;
}

bool CandidEncoder::encode(const CandidTypeGraph &g,
                           const std::vector<int> &args,
                           const std::vector<CandidValue> &values,
                           std::vector<uint8_t> &out, std::string &err,
                           const CandidDecodeLimits &limits) {
  if (args.size() != values.size()) {
    err = "There are " + std::to_string(args.size()) + " argument types and " +
          std::to_string(values.size()) + " values";
    return false;
  }
  std::vector<uint8_t> bytes;
  if (!encode_header(g, args, bytes, err, limits)) return false;
  ValueWriter w(g, bytes, limits);
  for (size_t i = 0; i < args.size(); ++i)
    if (!w.write(args[i], values[i])) {
      err = w.m_err;
      return false;
    }
  out.insert(out.end(), bytes.begin(), bytes.end());
  return true;
}
