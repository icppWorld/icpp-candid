// The Candid decoder: deserialisation at an expected type, as the spec defines
// it.

#include "candid_decoder.h"
#include "candid_subtype.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <map>
#include <memory>

namespace {

// The result of decoding one value at an expected type:
//   Ok      - decoded and coerced
//   Subtype - the value does not coerce to the expected type. Under an opt,
//             this turns into null; anywhere else it fails the message.
//   Fail    - the message is not valid Candid, or a limit was hit. Nothing
//             recovers from it.
enum class St { Ok, Subtype, Fail };

class Decoder {
public:
  Decoder(const uint8_t *data, size_t size, const CandidTypeGraph &expected,
          const CandidDecodeLimits &limits)
      : m_p(data), m_n(size), m_e(expected), m_limits(limits),
        m_sub(limits.max_depth) {}

  bool run(const std::vector<int> &args, std::vector<CandidValue> &out,
           std::string &err);
  bool types(CandidTypeGraph &wire, std::vector<int> &args, std::string &err);

private:
  const uint8_t *m_p;
  size_t m_n;
  size_t m_pos{0};
  const CandidTypeGraph &m_e;
  CandidDecodeLimits m_limits;
  CandidTypeGraph m_w; // the wire types
  std::vector<int> m_wargs;
  CandidSubtype m_sub;
  size_t m_zero_width{0};
  size_t m_depth{0};
  std::string m_err;
  // The rest of the records decoded so far (see CandidValue::rest), by wire
  // type and expected type. Values are made only at the expected types.
  std::map<std::pair<int, int>, std::shared_ptr<const CandidValue>> m_rest;

  St fail(const std::string &msg) {
    m_err = msg;
    return St::Fail;
  }
  St mismatch(const std::string &msg) {
    m_err = msg;
    return St::Subtype;
  }
  size_t left() const { return m_n - m_pos; }

  // --- Reading ---
  bool read_byte(uint8_t &b);
  bool read_uleb64(uint64_t &v);
  bool read_sleb64(int64_t &v);
  bool read_leb_groups(std::vector<uint8_t> &groups);
  bool read_len(uint64_t &v, const char *what);

  // --- The type table ---
  bool read_header();
  bool read_type_ref(int &t);
  bool read_entry(CandidGraphNode &n);

  // --- Values ---
  St decode(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St decode_(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St skip(int w) { return decode(w, m_w, w, nullptr); }
  St some(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St decode_vec(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St decode_record(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St decode_variant(int w, const CandidTypeGraph &g, int e, CandidValue *out);
  St read_principal(std::vector<uint8_t> *bytes);
  St read_text(std::string *text, const char *what);
  void null_value(const CandidTypeGraph &g, int e, CandidValue *out);
};

bool Decoder::read_byte(uint8_t &b) {
  if (m_pos >= m_n) return false;
  b = m_p[m_pos++];
  return true;
}

bool Decoder::read_uleb64(uint64_t &v) {
  v = 0;
  unsigned shift = 0;
  while (true) {
    uint8_t b;
    if (!read_byte(b)) return false;
    uint64_t g = b & 0x7f;
    if (shift < 64) {
      // The bits that do not fit in 64 must be zero
      if (shift > 57 && (g >> (64 - shift)) != 0) return false;
      v |= g << shift;
    } else if (g != 0) {
      return false;
    }
    if ((b & 0x80) == 0) return true;
    shift += 7;
  }
}

bool Decoder::read_sleb64(int64_t &v) {
  std::vector<uint8_t> groups;
  if (!read_leb_groups(groups)) return false;
  CandidBigInt big = CandidBigInt::from_sleb_groups(groups);
  return big.to_i64(v);
}

bool Decoder::read_leb_groups(std::vector<uint8_t> &groups) {
  groups.clear();
  while (true) {
    uint8_t b;
    if (!read_byte(b)) return false;
    groups.push_back(b & 0x7f);
    if ((b & 0x80) == 0) return true;
  }
}

// A count or a length, which must not be more than the bytes left: every
// element it announces takes at least one byte
bool Decoder::read_len(uint64_t &v, const char *what) {
  if (!read_uleb64(v)) {
    m_err = std::string("Cannot read the ") + what;
    return false;
  }
  if (v > left()) {
    m_err = std::string("The ") + what + " is larger than the bytes left";
    return false;
  }
  return true;
}

bool Decoder::read_type_ref(int &t) {
  int64_t v;
  if (!read_sleb64(v)) {
    m_err = "Cannot read a type reference";
    return false;
  }
  CandidKind k;
  if (v >= 0) {
    // Checked against the table size once the whole table is read
    if (v > INT_MAX) {
      m_err = "Type index out of range";
      return false;
    }
    t = int(v);
    return true;
  }
  if (v < INT_MIN || !CandidTypeGraph::prim_kind(int(v), k)) {
    m_err = "Not a primitive type: opcode " + std::to_string(v);
    return false;
  }
  t = int(v);
  return true;
}

bool Decoder::read_entry(CandidGraphNode &n) {
  int64_t op;
  if (!read_sleb64(op)) {
    m_err = "Cannot read a type table opcode";
    return false;
  }
  switch (op) {
  case -18: // opt
  case -19: // vec
    n.kind = op == -18 ? CandidKind::Opt : CandidKind::Vec;
    return read_type_ref(n.inner);

  case -20:   // record
  case -21: { // variant
    n.kind = op == -20 ? CandidKind::Record : CandidKind::Variant;
    uint64_t count;
    if (!read_len(count, "number of fields")) return false;
    for (uint64_t i = 0; i < count; ++i) {
      uint64_t id;
      if (!read_uleb64(id)) {
        m_err = "Cannot read a field id";
        return false;
      }
      if (id > UINT32_MAX) {
        m_err = "Field id larger than 32 bits";
        return false;
      }
      if (i > 0 && id <= n.fields.back().id) {
        m_err = "Field ids are not sorted, or not unique";
        return false;
      }
      CandidGraphField f;
      f.id = uint32_t(id);
      if (!read_type_ref(f.type)) return false;
      n.fields.push_back(f);
    }
    return true;
  }

  case -22: { // func
    n.kind = CandidKind::Func;
    for (std::vector<int> *list : {&n.args, &n.rets}) {
      uint64_t count;
      if (!read_len(count, "number of function arguments")) return false;
      for (uint64_t i = 0; i < count; ++i) {
        int t;
        if (!read_type_ref(t)) return false;
        list->push_back(t);
      }
    }
    uint64_t count;
    if (!read_len(count, "number of function annotations")) return false;
    for (uint64_t i = 0; i < count; ++i) {
      uint8_t a;
      if (!read_byte(a)) {
        m_err = "Cannot read a function annotation";
        return false;
      }
      if (a != CANDID_FUNC_QUERY && a != CANDID_FUNC_ONEWAY &&
          a != CANDID_FUNC_COMPOSITE_QUERY) {
        m_err = "Unknown function annotation " + std::to_string(a);
        return false;
      }
      n.modes.push_back(a);
    }
    std::sort(n.modes.begin(), n.modes.end());
    return true;
  }

  case -23: { // service
    n.kind = CandidKind::Service;
    uint64_t count;
    if (!read_len(count, "number of methods")) return false;
    for (uint64_t i = 0; i < count; ++i) {
      CandidGraphMethod m;
      if (read_text(&m.name, "method name") != St::Ok) return false;
      if (i > 0 && m.name <= n.methods.back().name) {
        m_err = "Method names are not sorted, or not unique";
        return false;
      }
      if (!read_type_ref(m.type)) return false;
      n.methods.push_back(m);
    }
    return true;
  }

  default:
    if (op < -24) {
      // A future type: skipped by its declared size
      n.kind = CandidKind::Future;
      uint64_t len;
      if (!read_len(len, "future type length")) return false;
      m_pos += size_t(len);
      return true;
    }
    m_err =
        "Type table entry " + std::to_string(op) + " is not a type constructor";
    return false;
  }
}

bool Decoder::read_header() {
  if (m_n < 4 || std::memcmp(m_p, "DIDL", 4) != 0) {
    m_err = "Not a Candid message: it does not start with DIDL";
    return false;
  }
  m_pos = 4;

  uint64_t count;
  if (!read_len(count, "type table length")) return false;
  if (count > m_limits.max_type_table) {
    m_err = "The type table has more than " +
            std::to_string(m_limits.max_type_table) + " entries";
    return false;
  }
  for (uint64_t i = 0; i < count; ++i) {
    CandidGraphNode n;
    if (!read_entry(n)) return false;
    m_w.add(std::move(n));
  }

  // References can point forward, so they are checked once all are read
  auto ok = [&](int t) {
    if (!m_w.is_valid_ref(t)) {
      m_err = "Type index " + std::to_string(t) + " out of range";
      return false;
    }
    return true;
  };
  for (size_t i = 0; i < m_w.size(); ++i) {
    const CandidGraphNode &n = m_w.node(int(i));
    if ((n.kind == CandidKind::Opt || n.kind == CandidKind::Vec) &&
        !ok(n.inner))
      return false;
    for (const auto &f : n.fields)
      if (!ok(f.type)) return false;
    for (int t : n.args)
      if (!ok(t)) return false;
    for (int t : n.rets)
      if (!ok(t)) return false;
    for (const auto &m : n.methods) {
      if (!ok(m.type)) return false;
      if (m.type < 0 || m_w.node(m.type).kind != CandidKind::Func) {
        m_err = "Method " + m.name + " has a type that is not a function";
        return false;
      }
    }
  }

  // Records that can only contain themselves have no values
  m_w.replace_empty_records();

  if (!read_len(count, "number of arguments")) return false;
  for (uint64_t i = 0; i < count; ++i) {
    int t;
    if (!read_type_ref(t) || !ok(t)) return false;
    m_wargs.push_back(t);
  }
  return true;
}

St Decoder::read_text(std::string *text, const char *what) {
  uint64_t len;
  if (!read_len(len, what)) return St::Fail;
  const uint8_t *s = m_p + m_pos;
  if (!CandidDecoder::is_valid_utf8(s, size_t(len)))
    return fail(std::string("Invalid UTF-8 in a ") + what);
  if (text) text->assign(reinterpret_cast<const char *>(s), size_t(len));
  m_pos += size_t(len);
  return St::Ok;
}

St Decoder::read_principal(std::vector<uint8_t> *bytes) {
  uint8_t flag;
  if (!read_byte(flag)) return fail("Cannot read a principal");
  if (flag == 0) return fail("Opaque reference not supported");
  if (flag != 1) return fail("Invalid principal flag");
  uint64_t len;
  if (!read_len(len, "principal length")) return St::Fail;
  if (len > 29) return fail("A principal is at most 29 bytes");
  if (bytes) bytes->assign(m_p + m_pos, m_p + m_pos + len);
  m_pos += size_t(len);
  return St::Ok;
}

void Decoder::null_value(const CandidTypeGraph &g, int e, CandidValue *out) {
  if (!out) return;
  *out = CandidValue();
  out->kind = g.kind(e); // null, opt (null) or reserved
}

St Decoder::decode(int w, const CandidTypeGraph &g, int e, CandidValue *out) {
  if (m_depth >= m_limits.max_depth)
    return fail("Values nest more than " + std::to_string(m_limits.max_depth) +
                " deep");
  ++m_depth;
  size_t start = m_pos;
  St st = decode_(w, g, e, out);
  --m_depth;
  if (st == St::Ok && m_pos == start &&
      ++m_zero_width > m_limits.max_zero_width_values)
    return fail("More than " + std::to_string(m_limits.max_zero_width_values) +
                " values that take no bytes");
  return st;
}

// opt <v> : opt <t> ~> opt <v'> : opt <t'>, or null when v does not coerce
St Decoder::some(int w, const CandidTypeGraph &g, int e, CandidValue *out) {
  size_t save = m_pos;
  CandidValue inner;
  St st = decode(w, g, e, out ? &inner : nullptr);
  if (st == St::Ok) {
    if (out) out->items.push_back(std::move(inner));
    return St::Ok;
  }
  if (st == St::Fail) return St::Fail;
  // It does not coerce: read past the value, which must still be valid
  m_pos = save;
  st = skip(w);
  if (st != St::Ok) return St::Fail;
  return St::Ok; // null
}

St Decoder::decode_(int w, const CandidTypeGraph &g, int e, CandidValue *out) {
  CandidKind wk = m_w.kind(w);
  CandidKind ek = g.kind(e);
  if (out) {
    *out = CandidValue();
    out->kind = ek;
  }

  switch (ek) {
  case CandidKind::Reserved:
    // _ : <t> ~> null : reserved. The value is still read and checked, as
    // part of this value: decode() already counts it, if it takes no bytes
    if (wk == CandidKind::Reserved) return St::Ok;
    return decode_(w, m_w, w, nullptr);

  case CandidKind::Empty:
    if (wk == CandidKind::Empty)
      return fail("Cannot decode a value of type empty");
    return mismatch("Nothing coerces to empty");

  case CandidKind::Opt: {
    int ei = g.node(e).inner;
    if (wk == CandidKind::Null || wk == CandidKind::Reserved) return St::Ok;
    if (wk == CandidKind::Opt) {
      uint8_t flag;
      if (!read_byte(flag)) return fail("Cannot read an opt");
      if (flag == 0) return St::Ok;
      if (flag != 1) return fail("Invalid opt flag " + std::to_string(flag));
      return some(m_w.node(w).inner, g, ei, out);
    }
    // A value that is not null, opt or reserved, at an opt: coerced at the
    // constituent type. At an infinite option that is undefined.
    if (g.is_infinite_opt(e))
      return fail("Coercion at an infinite option is undefined");
    return some(w, g, ei, out);
  }

  case CandidKind::Null:
    if (wk != CandidKind::Null) return mismatch("Expected null");
    return St::Ok;

  case CandidKind::Bool: {
    if (wk != CandidKind::Bool) return mismatch("Expected bool");
    uint8_t b;
    if (!read_byte(b)) return fail("Cannot read a bool");
    if (b > 1) return fail("A bool must be 0 or 1");
    if (out) out->b = b == 1;
    return St::Ok;
  }

  case CandidKind::Nat:
  case CandidKind::Int: {
    bool from_nat = wk == CandidKind::Nat;
    if (!(wk == ek || (from_nat && ek == CandidKind::Int)))
      return mismatch(ek == CandidKind::Nat ? "Expected nat" : "Expected int");
    std::vector<uint8_t> groups;
    if (!read_leb_groups(groups)) return fail("Cannot read a nat or int");
    if (out)
      out->n = from_nat ? CandidBigInt::from_leb_groups(groups)
                        : CandidBigInt::from_sleb_groups(groups);
    return St::Ok;
  }

  case CandidKind::Nat8:
  case CandidKind::Nat16:
  case CandidKind::Nat32:
  case CandidKind::Nat64:
  case CandidKind::Int8:
  case CandidKind::Int16:
  case CandidKind::Int32:
  case CandidKind::Int64:
  case CandidKind::Float32:
  case CandidKind::Float64: {
    if (wk != ek) return mismatch("Fixed-width types must match exactly");
    size_t width = 0;
    CandidTypeGraph::is_fixed_width(ek, width);
    if (left() < width) return fail("Not enough bytes for a fixed-width value");
    uint64_t raw = 0;
    for (size_t k = 0; k < width; ++k)
      raw |= uint64_t(m_p[m_pos + k]) << (8 * k);
    m_pos += width;
    if (out) {
      switch (ek) {
      case CandidKind::Int8:
        out->i = int8_t(raw);
        break;
      case CandidKind::Int16:
        out->i = int16_t(raw);
        break;
      case CandidKind::Int32:
        out->i = int32_t(raw);
        break;
      case CandidKind::Int64:
        out->i = int64_t(raw);
        break;
      case CandidKind::Float32: {
        uint32_t r32 = uint32_t(raw);
        float f;
        std::memcpy(&f, &r32, 4);
        out->f64 = f;
        break;
      }
      case CandidKind::Float64:
        std::memcpy(&out->f64, &raw, 8);
        break;
      default:
        out->u = raw;
        break;
      }
    }
    return St::Ok;
  }

  case CandidKind::Text:
    if (wk != CandidKind::Text) return mismatch("Expected text");
    return read_text(out ? &out->text : nullptr, "text");

  case CandidKind::Principal:
    // service <: principal: both carry the principal's bytes
    if (wk != CandidKind::Principal && wk != CandidKind::Service)
      return mismatch("Expected principal");
    return read_principal(out ? &out->bytes : nullptr);

  case CandidKind::Vec:
    return decode_vec(w, g, e, out);
  case CandidKind::Record:
    return decode_record(w, g, e, out);
  case CandidKind::Variant:
    return decode_variant(w, g, e, out);

  case CandidKind::Func: {
    if (wk != CandidKind::Func) return mismatch("Expected func");
    if (!m_sub.is_subtype(m_w, w, g, e))
      return mismatch("The function type is not a subtype of the expected one");
    uint8_t flag;
    if (!read_byte(flag)) return fail("Cannot read a func");
    if (flag == 0) return fail("Opaque reference not supported");
    if (flag != 1) return fail("Invalid func flag");
    St st = read_principal(out ? &out->bytes : nullptr);
    if (st != St::Ok) return st;
    return read_text(out ? &out->text : nullptr, "method name");
  }

  case CandidKind::Service:
    if (wk != CandidKind::Service) return mismatch("Expected service");
    if (!m_sub.is_subtype(m_w, w, g, e))
      return mismatch("The service type is not a subtype of the expected one");
    return read_principal(out ? &out->bytes : nullptr);

  case CandidKind::Future: {
    // Only when skipping a value of a future type: two counts m and n, then
    // m bytes (and n references, which this transport does not carry)
    uint64_t m, n;
    if (!read_len(m, "future value length")) return St::Fail;
    if (!read_uleb64(n)) return fail("Cannot read a future value");
    if (left() < m)
      return fail("The future value length is larger than the bytes left");
    m_pos += size_t(m);
    return St::Ok;
  }
  }
  return fail("Unknown type");
}

St Decoder::decode_vec(int w, const CandidTypeGraph &g, int e,
                       CandidValue *out) {
  if (m_w.kind(w) != CandidKind::Vec) return mismatch("Expected vec");
  int wi = m_w.node(w).inner;
  int ei = g.node(e).inner;
  uint64_t len;
  if (!read_uleb64(len)) return fail("Cannot read the vec length");

  if (m_w.is_zero_width(wi)) {
    // Its elements take no bytes, so the length is checked against the
    // budget for such values, before decoding any of them
    if (len > m_limits.max_zero_width_values -
                  std::min(m_zero_width, m_limits.max_zero_width_values))
      return fail("A vec of more than " +
                  std::to_string(m_limits.max_zero_width_values) +
                  " values that take no bytes");
  } else if (len > left()) {
    return fail("The vec length is larger than the bytes left");
  }

  // Elements of a fixed width, of the same type on both sides: read them as
  // a block, packed
  CandidKind wik = m_w.kind(wi);
  CandidKind eik = g.kind(ei);
  size_t width = 0;
  if (CandidTypeGraph::is_fixed_width(eik, width) && wik == eik) {
    if (len > left() / width) return fail("Not enough bytes for the vec");
    size_t nbytes = size_t(len) * width;
    const uint8_t *s = m_p + m_pos;
    if (eik == CandidKind::Bool) {
      for (size_t k = 0; k < nbytes; ++k)
        if (s[k] > 1) return fail("A bool must be 0 or 1");
    }
    if (out) {
      out->packed = true;
      out->elem_kind = eik;
      out->bytes.assign(s, s + nbytes);
    }
    m_pos += nbytes;
    return St::Ok;
  }
  if (out && CandidTypeGraph::is_fixed_width(eik, width)) {
    // The expected elements are packed; the wire ones do not match, so the
    // first element, if any, decides the outcome
    out->packed = true;
    out->elem_kind = eik;
    if (len == 0) return St::Ok;
    CandidValue item;
    St st = decode(wi, g, ei, &item);
    if (st == St::Ok) return fail("Internal error: packed vec element");
    return st;
  }

  // Elements that take no bytes all decode to the same value: keep the
  // first, and only check the others, which counts them against the budget
  bool repeat = m_w.is_zero_width(wi);
  // No room is reserved for the length the message claims: vecs nest, and
  // each would reserve for nearly all the bytes left. The elements that are
  // there make the room for themselves.
  for (uint64_t k = 0; k < len; ++k) {
    bool keep = out && (!repeat || k == 0);
    CandidValue item;
    St st = decode(wi, g, ei, keep ? &item : nullptr);
    if (st != St::Ok) return st;
    if (keep) out->items.push_back(std::move(item));
  }
  if (out && repeat && len > 0) out->repeat = size_t(len);
  return St::Ok;
}

St Decoder::decode_record(int w, const CandidTypeGraph &g, int e,
                          CandidValue *out) {
  if (m_w.kind(w) != CandidKind::Record) return mismatch("Expected record");
  const auto &wfs = m_w.node(w).fields;
  const auto &efs = g.node(e).fields;

  // The fields that take no bytes on the wire and the missing ones go in the
  // rest, which is made the first time, and shared after that. They are
  // still decoded every time: that checks them, and counts them against the
  // budget for values that take no bytes.
  auto cached = m_rest.find({w, e});
  std::shared_ptr<CandidValue> rest;
  if (out && cached == m_rest.end()) {
    rest = std::make_shared<CandidValue>();
    rest->kind = CandidKind::Record;
  }
  auto slot = [&](uint32_t id, bool in_rest) -> CandidValue * {
    CandidValue *r = in_rest ? rest.get() : out;
    if (!r) return nullptr;
    r->ids.push_back(id);
    r->items.emplace_back();
    return &r->items.back();
  };

  // Both are sorted by id: walk them together
  size_t j = 0;
  auto missing = [&](size_t idx) -> St {
    // A field only in the expected type must be null, opt or reserved, or be
    // marked optional
    if (efs[idx].optional) {
      if (CandidValue *v = slot(efs[idx].id, true)) v->absent = true;
      return St::Ok;
    }
    if (!g.is_null_like(efs[idx].type))
      return mismatch("Record field " + std::to_string(efs[idx].id) +
                      " is missing");
    null_value(g, efs[idx].type, slot(efs[idx].id, true));
    return St::Ok;
  };
  for (const auto &wf : wfs) {
    while (j < efs.size() && efs[j].id < wf.id) {
      St st = missing(j++);
      if (st != St::Ok) return st;
    }
    St st;
    if (j < efs.size() && efs[j].id == wf.id) {
      st = decode(wf.type, g, efs[j].type,
                  slot(wf.id, m_w.is_zero_width(wf.type)));
      ++j;
    } else {
      st = skip(wf.type); // a field only on the wire is read and ignored
    }
    if (st != St::Ok) return st;
  }
  while (j < efs.size()) {
    St st = missing(j++);
    if (st != St::Ok) return st;
  }

  if (out) {
    std::shared_ptr<const CandidValue> r =
        rest ? m_rest.emplace(std::make_pair(w, e), rest).first->second
             : cached->second;
    if (!r->ids.empty()) out->rest = std::move(r);
  }
  return St::Ok;
}

St Decoder::decode_variant(int w, const CandidTypeGraph &g, int e,
                           CandidValue *out) {
  if (m_w.kind(w) != CandidKind::Variant) return mismatch("Expected variant");
  const auto &wfs = m_w.node(w).fields;
  uint64_t idx;
  if (!read_uleb64(idx)) return fail("Cannot read the variant index");
  if (idx >= wfs.size()) return fail("Variant index out of range");
  const CandidGraphField &wf = wfs[size_t(idx)];
  const auto &efs = g.node(e).fields;
  auto it = std::lower_bound(
      efs.begin(), efs.end(), wf.id,
      [](const CandidGraphField &f, uint32_t v) { return f.id < v; });
  if (it == efs.end() || it->id != wf.id)
    return mismatch("Variant case " + std::to_string(wf.id) +
                    " is not in the expected type");
  CandidValue item;
  St st = decode(wf.type, g, it->type, out ? &item : nullptr);
  if (st != St::Ok) return st;
  if (out) {
    out->ids.push_back(wf.id);
    out->items.push_back(std::move(item));
  }
  return St::Ok;
}

bool Decoder::run(const std::vector<int> &args, std::vector<CandidValue> &out,
                  std::string &err) {
  out.clear();
  if (!read_header()) {
    err = m_err;
    return false;
  }
  out.resize(args.size());
  for (size_t i = 0; i < args.size(); ++i) {
    if (i < m_wargs.size()) {
      if (decode(m_wargs[i], m_e, args[i], &out[i]) != St::Ok) {
        err = "Argument " + std::to_string(i) + ": " + m_err;
        return false;
      }
    } else if (m_e.is_null_like(args[i])) {
      null_value(m_e, args[i], &out[i]);
    } else {
      err = "Argument " + std::to_string(i) +
            " is missing, and its type is not null, opt or reserved";
      return false;
    }
  }
  // Extra arguments are read and ignored
  for (size_t j = args.size(); j < m_wargs.size(); ++j) {
    if (skip(m_wargs[j]) != St::Ok) {
      err = "Extra argument " + std::to_string(j) + ": " + m_err;
      return false;
    }
  }
  if (m_pos != m_n) {
    err = "Trailing bytes after the last value";
    return false;
  }
  return true;
}

bool Decoder::types(CandidTypeGraph &wire, std::vector<int> &args,
                    std::string &err) {
  if (!read_header()) {
    err = m_err;
    return false;
  }
  wire = m_w;
  args = m_wargs;
  return true;
}

} // namespace

bool CandidDecoder::read_types(const uint8_t *data, size_t size,
                               CandidTypeGraph &wire, std::vector<int> &args,
                               std::string &err,
                               const CandidDecodeLimits &limits) {
  CandidTypeGraph none;
  Decoder d(data, size, none, limits);
  return d.types(wire, args, err);
}

bool CandidDecoder::decode(const uint8_t *data, size_t size,
                           const CandidTypeGraph &expected,
                           const std::vector<int> &args,
                           std::vector<CandidValue> &out, std::string &err,
                           const CandidDecodeLimits &limits) {
  if (!expected.validate(err)) {
    err = "The expected types are not valid: " + err;
    return false;
  }
  for (int t : args) {
    if (!expected.is_valid_ref(t)) {
      err = "An expected argument type is out of range";
      return false;
    }
  }
  Decoder d(data, size, expected, limits);
  return d.run(args, out, err);
}

bool CandidDecoder::is_valid_utf8(const uint8_t *s, size_t n) {
  size_t i = 0;
  while (i < n) {
    uint8_t c = s[i];
    if (c < 0x80) {
      ++i;
      continue;
    }
    size_t len;
    uint32_t cp;
    if (c >= 0xc2 && c <= 0xdf) {
      len = 2;
      cp = c & 0x1f;
    } else if (c >= 0xe0 && c <= 0xef) {
      len = 3;
      cp = c & 0x0f;
    } else if (c >= 0xf0 && c <= 0xf4) {
      len = 4;
      cp = c & 0x07;
    } else {
      return false;
    }
    if (n - i < len) return false;
    for (size_t k = 1; k < len; ++k) {
      uint8_t cc = s[i + k];
      if ((cc & 0xc0) != 0x80) return false;
      cp = (cp << 6) | (cc & 0x3f);
    }
    // No overlong forms, no surrogates, nothing above U+10FFFF
    if ((len == 3 && cp < 0x800) || (len == 4 && cp < 0x10000) ||
        (cp >= 0xd800 && cp <= 0xdfff) || cp > 0x10ffff)
      return false;
    i += len;
  }
  return true;
}
