// A decoded Candid value, at an expected type.

#include "candid_value.h"
#include "candid_principal_text.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

size_t CandidValue::vec_size() const {
  if (repeat) return repeat;
  if (!packed) return items.size();
  size_t w = 1;
  CandidTypeGraph::is_fixed_width(elem_kind, w);
  return bytes.size() / w;
}

CandidValue CandidValue::packed_element(size_t idx) const {
  CandidValue v;
  v.kind = elem_kind;
  size_t w = 1;
  CandidTypeGraph::is_fixed_width(elem_kind, w);
  uint64_t raw = 0;
  for (size_t k = 0; k < w; ++k)
    raw |= uint64_t(bytes[idx * w + k]) << (8 * k);
  switch (elem_kind) {
  case CandidKind::Bool:
    v.b = raw != 0;
    break;
  case CandidKind::Nat8:
  case CandidKind::Nat16:
  case CandidKind::Nat32:
  case CandidKind::Nat64:
    v.u = raw;
    break;
  case CandidKind::Int8:
    v.i = int8_t(raw);
    break;
  case CandidKind::Int16:
    v.i = int16_t(raw);
    break;
  case CandidKind::Int32:
    v.i = int32_t(raw);
    break;
  case CandidKind::Int64:
    v.i = int64_t(raw);
    break;
  case CandidKind::Float32: {
    uint32_t r32 = uint32_t(raw);
    float f;
    std::memcpy(&f, &r32, 4);
    v.f64 = f;
    break;
  }
  case CandidKind::Float64:
    std::memcpy(&v.f64, &raw, 8);
    break;
  default:
    break;
  }
  return v;
}

static const CandidValue *find_field(const std::vector<uint32_t> &ids,
                                     const std::vector<CandidValue> &items,
                                     uint32_t id) {
  auto it = std::lower_bound(ids.begin(), ids.end(), id);
  if (it == ids.end() || *it != id) return nullptr;
  return &items[size_t(it - ids.begin())];
}

const CandidValue *CandidValue::field(uint32_t id) const {
  if (const CandidValue *f = find_field(ids, items, id)) return f;
  return rest ? find_field(rest->ids, rest->items, id) : nullptr;
}

size_t CandidValue::field_count() const {
  return ids.size() + (rest ? rest->ids.size() : 0);
}

static std::vector<std::pair<uint32_t, const CandidValue *>>
fields_of(const CandidValue &v) {
  std::vector<std::pair<uint32_t, const CandidValue *>> fs;
  fs.reserve(v.field_count());
  v.for_each_field(
      [&](uint32_t id, const CandidValue &f) { fs.emplace_back(id, &f); });
  return fs;
}

bool CandidValue::operator==(const CandidValue &o) const {
  if (kind != o.kind || absent != o.absent) return false;
  switch (kind) {
  case CandidKind::Null:
  case CandidKind::Reserved:
  case CandidKind::Empty:
  case CandidKind::Future:
    return true;
  case CandidKind::Bool:
    return b == o.b;
  case CandidKind::Nat:
  case CandidKind::Int:
    return n == o.n;
  case CandidKind::Nat8:
  case CandidKind::Nat16:
  case CandidKind::Nat32:
  case CandidKind::Nat64:
    return u == o.u;
  case CandidKind::Int8:
  case CandidKind::Int16:
  case CandidKind::Int32:
  case CandidKind::Int64:
    return i == o.i;
  case CandidKind::Float32:
  case CandidKind::Float64:
    return std::memcmp(&f64, &o.f64, sizeof(double)) == 0;
  case CandidKind::Text:
    return text == o.text;
  case CandidKind::Principal:
  case CandidKind::Service:
    return bytes == o.bytes;
  case CandidKind::Func:
    return bytes == o.bytes && text == o.text;
  case CandidKind::Vec: {
    if (packed && o.packed) return elem_kind == o.elem_kind && bytes == o.bytes;
    size_t sz = vec_size();
    if (sz != o.vec_size()) return false;
    for (size_t k = 0; k < sz; ++k) {
      CandidValue a = packed ? packed_element(k) : at(k);
      CandidValue c = o.packed ? o.packed_element(k) : o.at(k);
      if (a != c) return false;
    }
    return true;
  }
  case CandidKind::Opt:
  case CandidKind::Variant:
    return ids == o.ids && items == o.items;
  case CandidKind::Record: {
    if (rest == o.rest) return ids == o.ids && items == o.items;
    auto a = fields_of(*this), c = fields_of(o);
    if (a.size() != c.size()) return false;
    for (size_t k = 0; k < a.size(); ++k)
      if (a[k].first != c[k].first || *a[k].second != *c[k].second)
        return false;
    return true;
  }
  }
  return false;
}

static void append_text_literal(std::string &out, const std::string &s) {
  out += '"';
  for (unsigned char c : s) {
    switch (c) {
    case '"':
      out += "\\\"";
      break;
    case '\\':
      out += "\\\\";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      if (c < 0x20 || c == 0x7f) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "\\%02x", c);
        out += buf;
      } else {
        out += char(c);
      }
    }
  }
  out += '"';
}

static void append_double(std::string &out, double d) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.17g", d);
  out += buf;
}

std::string CandidValue::to_text() const {
  std::string out;
  switch (kind) {
  case CandidKind::Null:
  case CandidKind::Reserved:
    out = "null";
    break;
  case CandidKind::Empty:
    out = "empty";
    break;
  case CandidKind::Future:
    out = "future";
    break;
  case CandidKind::Bool:
    out = b ? "true" : "false";
    break;
  case CandidKind::Nat:
  case CandidKind::Int:
    out = n.to_string();
    break;
  case CandidKind::Nat8:
  case CandidKind::Nat16:
  case CandidKind::Nat32:
  case CandidKind::Nat64:
    out = std::to_string(u);
    break;
  case CandidKind::Int8:
  case CandidKind::Int16:
  case CandidKind::Int32:
  case CandidKind::Int64:
    out = std::to_string(i);
    break;
  case CandidKind::Float32:
  case CandidKind::Float64:
    append_double(out, f64);
    break;
  case CandidKind::Text:
    append_text_literal(out, text);
    break;
  case CandidKind::Principal:
    out = "principal ";
    append_text_literal(out, CandidPrincipalText::encode(bytes));
    break;
  case CandidKind::Service:
    out = "service ";
    append_text_literal(out, CandidPrincipalText::encode(bytes));
    break;
  case CandidKind::Func:
    out = "func ";
    append_text_literal(out, CandidPrincipalText::encode(bytes));
    out += ".";
    append_text_literal(out, text);
    break;
  case CandidKind::Opt:
    out = items.empty() ? "null" : "opt " + items[0].to_text();
    break;
  case CandidKind::Vec: {
    if (packed && elem_kind == CandidKind::Nat8) {
      out = "blob \"";
      for (uint8_t c : bytes) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "\\%02x", c);
        out += buf;
      }
      out += "\"";
      break;
    }
    out = "vec {";
    size_t sz = vec_size();
    for (size_t k = 0; k < sz; ++k) {
      out += " ";
      out += packed ? packed_element(k).to_text() : at(k).to_text();
      out += ";";
    }
    out += " }";
    break;
  }
  case CandidKind::Record:
    out = "record {";
    for_each_field([&](uint32_t id, const CandidValue &f) {
      out += " " + std::to_string(id) + " = " + f.to_text() + ";";
    });
    out += " }";
    break;
  case CandidKind::Variant:
    out = "variant { " + std::to_string(ids[0]) + " = " + items[0].to_text() +
          " }";
    break;
  }
  return out;
}

std::string candid_values_to_text(const std::vector<CandidValue> &vs) {
  std::string out = "(";
  for (size_t k = 0; k < vs.size(); ++k) {
    if (k > 0) out += ", ";
    out += vs[k].to_text();
  }
  out += ")";
  return out;
}
