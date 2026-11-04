// Textual Candid values: parsed, and annotated at expected types.

#include "candid_text_value.h"
#include "candid_decoder.h"
#include "candid_principal_text.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace {

// A value as written, before it has a type
struct Raw {
  enum Kind {
    Null,
    Bool,
    Number, // an integer literal: text holds it, sign included
    Float,
    Text, // bytes
    Blob, // bytes
    Opt,
    Vec,
    Record,
    Variant,
    Principal, // bytes
    Service,   // bytes
    Func,      // bytes and text (the method)
  };
  Kind kind{Null};
  bool b{false};
  double f{0.0};
  std::string text;
  std::vector<uint8_t> bytes;
  std::vector<uint32_t> ids;
  std::vector<Raw> items;
};

constexpr size_t MAX_DEPTH = 128;

class Parser {
public:
  explicit Parser(const std::string &s) : m_s(s) {}

  bool args(std::vector<Raw> &out);
  std::string err() const {
    return m_err + " (at offset " + std::to_string(m_pos) + ")";
  }

private:
  const std::string &m_s;
  size_t m_pos{0};
  size_t m_depth{0};
  std::string m_err;

  bool fail(const std::string &msg) {
    if (m_err.empty()) m_err = msg;
    return false;
  }
  void ws();
  bool eat(char c);
  bool expect(char c);
  bool ident(std::string &id);
  bool keyword(const char *kw);
  bool string_lit(std::vector<uint8_t> &bytes);
  bool label(uint32_t &id, bool &have, bool variant);
  bool number(Raw &r);
  bool value(Raw &r);
  bool fields(Raw &r, bool variant);
};

bool is_ident_start(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
bool is_ident_char(char c) {
  return is_ident_start(c) || (c >= '0' && c <= '9');
}
int hex_digit(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

void Parser::ws() {
  while (m_pos < m_s.size()) {
    char c = m_s[m_pos];
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      ++m_pos;
    } else if (m_s.compare(m_pos, 2, "//") == 0) {
      while (m_pos < m_s.size() && m_s[m_pos] != '\n')
        ++m_pos;
    } else {
      return;
    }
  }
}

bool Parser::eat(char c) {
  ws();
  if (m_pos < m_s.size() && m_s[m_pos] == c) {
    ++m_pos;
    return true;
  }
  return false;
}

bool Parser::expect(char c) {
  if (eat(c)) return true;
  return fail(std::string("Expected '") + c + "'");
}

bool Parser::ident(std::string &id) {
  ws();
  if (m_pos >= m_s.size() || !is_ident_start(m_s[m_pos])) return false;
  size_t start = m_pos;
  while (m_pos < m_s.size() && is_ident_char(m_s[m_pos]))
    ++m_pos;
  id = m_s.substr(start, m_pos - start);
  return true;
}

// A keyword, not followed by more identifier characters
bool Parser::keyword(const char *kw) {
  ws();
  size_t n = std::strlen(kw);
  if (m_s.compare(m_pos, n, kw) != 0) return false;
  if (m_pos + n < m_s.size() && is_ident_char(m_s[m_pos + n])) return false;
  m_pos += n;
  return true;
}

static void append_utf8(std::vector<uint8_t> &out, uint32_t cp) {
  if (cp < 0x80) {
    out.push_back(uint8_t(cp));
  } else if (cp < 0x800) {
    out.push_back(uint8_t(0xc0 | (cp >> 6)));
    out.push_back(uint8_t(0x80 | (cp & 0x3f)));
  } else if (cp < 0x10000) {
    out.push_back(uint8_t(0xe0 | (cp >> 12)));
    out.push_back(uint8_t(0x80 | ((cp >> 6) & 0x3f)));
    out.push_back(uint8_t(0x80 | (cp & 0x3f)));
  } else {
    out.push_back(uint8_t(0xf0 | (cp >> 18)));
    out.push_back(uint8_t(0x80 | ((cp >> 12) & 0x3f)));
    out.push_back(uint8_t(0x80 | ((cp >> 6) & 0x3f)));
    out.push_back(uint8_t(0x80 | (cp & 0x3f)));
  }
}

// "..." with the spec's escapes: \t \n \r \" \' \\, \HH (a byte), \u{H_H}
bool Parser::string_lit(std::vector<uint8_t> &bytes) {
  ws();
  bytes.clear();
  if (m_pos >= m_s.size() || m_s[m_pos] != '"') return fail("Expected a text");
  ++m_pos;
  while (true) {
    if (m_pos >= m_s.size()) return fail("Unclosed text");
    char c = m_s[m_pos++];
    if (c == '"') return true;
    if (c != '\\') {
      bytes.push_back(uint8_t(c));
      continue;
    }
    if (m_pos >= m_s.size()) return fail("Unclosed text");
    char e = m_s[m_pos++];
    switch (e) {
    case 't':
      bytes.push_back('\t');
      break;
    case 'n':
      bytes.push_back('\n');
      break;
    case 'r':
      bytes.push_back('\r');
      break;
    case '"':
      bytes.push_back('"');
      break;
    case '\'':
      bytes.push_back('\'');
      break;
    case '\\':
      bytes.push_back('\\');
      break;
    case 'u': {
      if (m_pos >= m_s.size() || m_s[m_pos] != '{')
        return fail("Expected '{' after \\u");
      ++m_pos;
      uint32_t cp = 0;
      size_t digits = 0;
      while (true) {
        if (m_pos >= m_s.size()) return fail("Unclosed \\u{...}");
        char h = m_s[m_pos++];
        if (h == '}') break;
        if (h == '_' && digits > 0) continue;
        int d = hex_digit(h);
        if (d < 0) return fail("Invalid \\u{...} escape");
        if (++digits > 6) return fail("\\u{...} escape too long");
        cp = cp * 16 + uint32_t(d);
      }
      if (digits == 0 || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
        return fail("Invalid Unicode scalar value in \\u{...}");
      append_utf8(bytes, cp);
      break;
    }
    default: {
      int h1 = hex_digit(e);
      int h2 = m_pos < m_s.size() ? hex_digit(m_s[m_pos]) : -1;
      if (h1 < 0 || h2 < 0) return fail("Invalid escape in text");
      ++m_pos;
      bytes.push_back(uint8_t(h1 * 16 + h2));
    }
    }
  }
}

// An integer or float literal, with an optional sign; '_' separates digits
bool Parser::number(Raw &r) {
  ws();
  size_t start = m_pos;
  std::string digits;
  if (m_pos < m_s.size() && (m_s[m_pos] == '+' || m_s[m_pos] == '-'))
    digits += m_s[m_pos++];
  bool hex = m_s.compare(m_pos, 2, "0x") == 0;
  bool is_float = false;
  if (hex) {
    digits += "0x";
    m_pos += 2;
  }
  size_t ndigits = 0;
  while (m_pos < m_s.size()) {
    char c = m_s[m_pos];
    bool digit = hex ? hex_digit(c) >= 0 : (c >= '0' && c <= '9');
    if (digit) {
      digits += c;
      ++ndigits;
    } else if (c == '_' && ndigits > 0) {
      // separator
    } else if (!hex && (c == '.' || c == 'e' || c == 'E')) {
      is_float = true;
      digits += c;
      if ((c == 'e' || c == 'E') && m_pos + 1 < m_s.size() &&
          (m_s[m_pos + 1] == '+' || m_s[m_pos + 1] == '-'))
        digits += m_s[++m_pos];
    } else {
      break;
    }
    ++m_pos;
  }
  if (ndigits == 0) {
    m_pos = start;
    return fail("Expected a value");
  }
  if (!is_float) {
    r.kind = Raw::Number;
    r.text = digits;
    return true;
  }
  std::string s;
  for (char c : digits)
    if (c != '_') s += c;
  char *end = nullptr;
  errno = 0;
  double d = std::strtod(s.c_str(), &end);
  if (end != s.c_str() + s.size() || errno == ERANGE)
    return fail("Invalid float literal " + s);
  r.kind = Raw::Float;
  r.f = d;
  return true;
}

// A field label: a nat, a name, or a quoted name. `have` is false when there
// is no label here (a value follows directly).
bool Parser::label(uint32_t &id, bool &have, bool variant) {
  ws();
  have = false;
  size_t save = m_pos;
  std::string name;
  if (m_pos < m_s.size() && m_s[m_pos] >= '0' && m_s[m_pos] <= '9') {
    // A nat is a label when '=' follows, or, in a variant, when the case ends
    Raw r;
    if (!number(r) || r.kind != Raw::Number) {
      m_pos = save;
      m_err.clear();
      return true;
    }
    ws();
    char nx = m_pos < m_s.size() ? m_s[m_pos] : '\0';
    if (!(nx == '=' || (variant && (nx == '}' || nx == ';')))) {
      m_pos = save;
      return true;
    }
    CandidBigInt big;
    uint64_t v;
    if (!CandidBigInt::parse(r.text, big) || !big.to_u64(v) || v > UINT32_MAX)
      return fail("Field id out of range");
    id = uint32_t(v);
    have = true;
    return true;
  }
  if (m_pos < m_s.size() && m_s[m_pos] == '"') {
    std::vector<uint8_t> bytes;
    if (!string_lit(bytes)) return false;
    id = CandidTypeGraph::idl_hash(std::string(bytes.begin(), bytes.end()));
    have = true;
    return true;
  }
  if (ident(name)) {
    // Keywords that start a value are not labels unless '=' follows
    size_t after = m_pos;
    bool eq = eat('=');
    m_pos = after;
    static const char *kws[] = {"null",      "true",    "false",   "opt",
                                "vec",       "record",  "variant", "blob",
                                "principal", "service", "func"};
    bool is_kw =
        std::find_if(std::begin(kws), std::end(kws),
                     [&](const char *k) { return name == k; }) != std::end(kws);
    if (is_kw && !eq) {
      m_pos = save;
      return true;
    }
    id = CandidTypeGraph::idl_hash(name);
    have = true;
    return true;
  }
  return true;
}

// record { f;* } or variant { f }, where f is `label = value`, `value`
// (records: the next id), or `label` (variants: a case of type null)
bool Parser::fields(Raw &r, bool variant) {
  if (!expect('{')) return false;
  uint32_t next = 0;
  while (!eat('}')) {
    uint32_t id = 0;
    bool have = false;
    size_t save = m_pos;
    if (!label(id, have, variant)) return false;
    Raw v;
    if (have && eat('=')) {
      if (!value(v)) return false;
    } else if (have && variant) {
      v.kind = Raw::Null; // variant { name }
    } else {
      // A positional value: no label after all
      m_pos = save;
      if (variant) return fail("A variant case needs a label");
      id = next;
      if (!value(v)) return false;
    }
    if (std::find(r.ids.begin(), r.ids.end(), id) != r.ids.end())
      return fail("Duplicate field id " + std::to_string(id));
    r.ids.push_back(id);
    r.items.push_back(std::move(v));
    next = id + 1;
    if (!eat(';')) {
      if (!expect('}')) return false;
      break;
    }
  }
  if (variant && r.ids.size() != 1)
    return fail("A variant value has exactly one case");
  return true;
}

bool Parser::value(Raw &r) {
  if (m_depth >= MAX_DEPTH) return fail("Values nest too deep");
  ++m_depth;
  struct Leave {
    size_t &d;
    ~Leave() { --d; }
  } leave{m_depth};

  ws();
  if (m_pos >= m_s.size()) return fail("Expected a value");
  char c = m_s[m_pos];
  if (c == '"') {
    r.kind = Raw::Text;
    return string_lit(r.bytes);
  }
  if (c == '(') {
    // A parenthesised value
    ++m_pos;
    if (!value(r)) return false;
    return expect(')');
  }
  if (keyword("null")) {
    r.kind = Raw::Null;
    return true;
  }
  if (keyword("true")) {
    r.kind = Raw::Bool;
    r.b = true;
    return true;
  }
  if (keyword("false")) {
    r.kind = Raw::Bool;
    return true;
  }
  if (keyword("opt")) {
    r.kind = Raw::Opt;
    r.items.resize(1);
    return value(r.items[0]);
  }
  if (keyword("vec")) {
    r.kind = Raw::Vec;
    if (!expect('{')) return false;
    while (!eat('}')) {
      Raw v;
      if (!value(v)) return false;
      r.items.push_back(std::move(v));
      if (!eat(';')) return expect('}');
    }
    return true;
  }
  if (keyword("record")) {
    r.kind = Raw::Record;
    return fields(r, false);
  }
  if (keyword("variant")) {
    r.kind = Raw::Variant;
    return fields(r, true);
  }
  if (keyword("blob")) {
    r.kind = Raw::Blob;
    return string_lit(r.bytes);
  }
  bool is_principal = keyword("principal");
  if (is_principal || keyword("service")) {
    r.kind = is_principal ? Raw::Principal : Raw::Service;
    std::vector<uint8_t> t;
    if (!string_lit(t)) return false;
    std::string e;
    if (!CandidPrincipalText::decode(std::string(t.begin(), t.end()), r.bytes,
                                     e))
      return fail(e);
    return true;
  }
  if (keyword("func")) {
    r.kind = Raw::Func;
    std::vector<uint8_t> t;
    if (!string_lit(t)) return false;
    std::string e;
    if (!CandidPrincipalText::decode(std::string(t.begin(), t.end()), r.bytes,
                                     e))
      return fail(e);
    if (!expect('.')) return false;
    ws();
    if (m_pos < m_s.size() && m_s[m_pos] == '"') {
      std::vector<uint8_t> m;
      if (!string_lit(m)) return false;
      r.text.assign(m.begin(), m.end());
    } else if (!ident(r.text)) {
      return fail("Expected a method name");
    }
    return true;
  }
  return number(r);
}

bool Parser::args(std::vector<Raw> &out) {
  if (!expect('(')) return false;
  while (!eat(')')) {
    Raw v;
    if (!value(v)) return false;
    out.push_back(std::move(v));
    if (!eat(',')) {
      if (!expect(')')) return false;
      break;
    }
  }
  ws();
  if (m_pos != m_s.size()) return fail("Unexpected text after the values");
  return true;
}

// --- Annotation ---

class Annotator {
public:
  explicit Annotator(const CandidTypeGraph &g) : m_g(g) {}
  bool annotate(const Raw &r, int t, CandidValue &out);
  std::string m_err;

private:
  const CandidTypeGraph &m_g;
  size_t m_depth{0};

  bool fail(const std::string &msg) {
    if (m_err.empty()) m_err = msg;
    return false;
  }
  bool number_at(const Raw &r, CandidKind k, CandidValue &out);
  bool annotate_vec(const Raw &r, int t, CandidValue &out);
};

void pack(CandidValue &vec, const CandidValue &e) {
  size_t width = 0;
  CandidTypeGraph::is_fixed_width(e.kind, width);
  uint64_t raw = 0;
  switch (e.kind) {
  case CandidKind::Bool:
    raw = e.b ? 1 : 0;
    break;
  case CandidKind::Int8:
  case CandidKind::Int16:
  case CandidKind::Int32:
  case CandidKind::Int64:
    raw = uint64_t(e.i);
    break;
  case CandidKind::Float32: {
    float f = float(e.f64);
    uint32_t r32;
    std::memcpy(&r32, &f, 4);
    raw = r32;
    break;
  }
  case CandidKind::Float64:
    std::memcpy(&raw, &e.f64, 8);
    break;
  default:
    raw = e.u;
    break;
  }
  for (size_t k = 0; k < width; ++k)
    vec.bytes.push_back(uint8_t(raw >> (8 * k)));
}

bool Annotator::number_at(const Raw &r, CandidKind k, CandidValue &out) {
  CandidBigInt big;
  if (!CandidBigInt::parse(r.text, big))
    return fail("Invalid number " + r.text);
  switch (k) {
  case CandidKind::Nat:
    if (big.is_negative()) return fail(r.text + " is not a nat");
    out.n = big;
    return true;
  case CandidKind::Int:
    out.n = big;
    return true;
  case CandidKind::Nat8:
  case CandidKind::Nat16:
  case CandidKind::Nat32:
  case CandidKind::Nat64: {
    size_t w = 0;
    CandidTypeGraph::is_fixed_width(k, w);
    uint64_t v;
    if (big.is_negative() || !big.to_u64(v) || (w < 8 && v >> (8 * w) != 0))
      return fail(r.text + " is out of range");
    out.u = v;
    return true;
  }
  case CandidKind::Int8:
  case CandidKind::Int16:
  case CandidKind::Int32:
  case CandidKind::Int64: {
    size_t w = 0;
    CandidTypeGraph::is_fixed_width(k, w);
    int64_t v;
    if (!big.to_i64(v)) return fail(r.text + " is out of range");
    if (w < 8) {
      int64_t lim = int64_t(1) << (8 * w - 1);
      if (v < -lim || v >= lim) return fail(r.text + " is out of range");
    }
    out.i = v;
    return true;
  }
  default:
    return fail("A number is not a value of type " +
                m_g.to_string(CandidTypeGraph::prim(k)));
  }
}

bool Annotator::annotate_vec(const Raw &r, int t, CandidValue &out) {
  int et = m_g.node(t).inner;
  CandidKind ek = m_g.kind(et);
  size_t width = 0;
  bool fixed = CandidTypeGraph::is_fixed_width(ek, width);
  if (fixed) {
    out.packed = true;
    out.elem_kind = ek;
  }
  if (r.kind == Raw::Blob) {
    if (ek != CandidKind::Nat8) return fail("A blob is a vec nat8");
    out.bytes = r.bytes;
    return true;
  }
  if (r.kind != Raw::Vec) return fail("Expected a vec");
  for (const Raw &item : r.items) {
    CandidValue v;
    if (!annotate(item, et, v)) return false;
    if (fixed) pack(out, v);
    else out.items.push_back(std::move(v));
  }
  return true;
}

bool Annotator::annotate(const Raw &r, int t, CandidValue &out) {
  if (m_depth >= MAX_DEPTH) return fail("Values nest too deep");
  ++m_depth;
  struct Leave {
    size_t &d;
    ~Leave() { --d; }
  } leave{m_depth};

  CandidKind k = m_g.kind(t);
  out = CandidValue();
  out.kind = k;
  auto mismatch = [&]() {
    return fail("Type mismatch: the value is not of type " + m_g.to_string(t));
  };

  switch (k) {
  case CandidKind::Reserved:
    return true;
  case CandidKind::Null:
    return r.kind == Raw::Null ? true : mismatch();
  case CandidKind::Opt:
    if (r.kind == Raw::Null) return true;
    if (r.kind != Raw::Opt) return mismatch();
    out.items.resize(1);
    return annotate(r.items[0], m_g.node(t).inner, out.items[0]);
  case CandidKind::Bool:
    if (r.kind != Raw::Bool) return mismatch();
    out.b = r.b;
    return true;
  case CandidKind::Float32:
  case CandidKind::Float64: {
    // A float literal, or a decimal integer literal (to_text writes 1.0 as 1)
    double d = r.f;
    if (r.kind == Raw::Number && r.text.find('x') == std::string::npos) {
      char *end = nullptr;
      errno = 0;
      d = std::strtod(r.text.c_str(), &end);
      if (end != r.text.c_str() + r.text.size() || errno == ERANGE)
        return fail("Invalid float literal " + r.text);
    } else if (r.kind != Raw::Float) {
      return mismatch();
    }
    out.f64 = k == CandidKind::Float32 ? double(float(d)) : d;
    return true;
  }
  case CandidKind::Text:
    if (r.kind != Raw::Text) return mismatch();
    if (!CandidDecoder::is_valid_utf8(r.bytes.data(), r.bytes.size()))
      return fail("Invalid UTF-8 in a text");
    out.text.assign(r.bytes.begin(), r.bytes.end());
    return true;
  case CandidKind::Principal:
    if (r.kind != Raw::Principal) return mismatch();
    out.bytes = r.bytes;
    return true;
  case CandidKind::Service:
    if (r.kind != Raw::Service) return mismatch();
    out.bytes = r.bytes;
    return true;
  case CandidKind::Func:
    if (r.kind != Raw::Func) return mismatch();
    if (!CandidDecoder::is_valid_utf8(
            reinterpret_cast<const uint8_t *>(r.text.data()), r.text.size()))
      return fail("Invalid UTF-8 in a method name");
    out.bytes = r.bytes;
    out.text = r.text;
    return true;
  case CandidKind::Vec:
    return annotate_vec(r, t, out);
  case CandidKind::Record: {
    if (r.kind != Raw::Record) return mismatch();
    for (const auto &f : m_g.node(t).fields) {
      auto it = std::find(r.ids.begin(), r.ids.end(), f.id);
      CandidValue v;
      if (it != r.ids.end()) {
        if (!annotate(r.items[size_t(it - r.ids.begin())], f.type, v))
          return false;
      } else if (m_g.is_null_like(f.type)) {
        v.kind = m_g.kind(f.type);
      } else {
        return fail("Record field " + std::to_string(f.id) + " not found");
      }
      out.ids.push_back(f.id);
      out.items.push_back(std::move(v));
    }
    return true;
  }
  case CandidKind::Variant: {
    if (r.kind != Raw::Variant) return mismatch();
    const auto &fs = m_g.node(t).fields;
    auto it =
        std::find_if(fs.begin(), fs.end(), [&](const CandidGraphField &f) {
          return f.id == r.ids[0];
        });
    if (it == fs.end())
      return fail("Variant case " + std::to_string(r.ids[0]) + " not found");
    CandidValue v;
    if (!annotate(r.items[0], it->type, v)) return false;
    out.ids.push_back(it->id);
    out.items.push_back(std::move(v));
    return true;
  }
  case CandidKind::Empty:
    return fail("No value has type empty");
  case CandidKind::Future:
    return fail("No textual value has a future type");
  default:
    // nat, int and the fixed-width integers
    if (r.kind != Raw::Number) return mismatch();
    return number_at(r, k, out);
  }
}

} // namespace

bool CandidTextValue::parse(const std::string &text, const CandidTypeGraph &g,
                            const std::vector<int> &args,
                            std::vector<CandidValue> &out, std::string &err) {
  out.clear();
  if (!g.validate(err)) {
    err = "The expected types are not valid: " + err;
    return false;
  }
  Parser p(text);
  std::vector<Raw> raws;
  if (!p.args(raws)) {
    err = p.err();
    return false;
  }
  Annotator a(g);
  out.resize(args.size());
  for (size_t i = 0; i < args.size(); ++i) {
    if (i < raws.size()) {
      if (!a.annotate(raws[i], args[i], out[i])) {
        err = "Argument " + std::to_string(i) + ": " + a.m_err;
        return false;
      }
    } else if (g.is_null_like(args[i])) {
      out[i] = CandidValue();
      out[i].kind = g.kind(args[i]);
    } else {
      err = "Argument " + std::to_string(i) +
            " is missing, and its type is not null, opt or reserved";
      return false;
    }
  }
  return true;
}
