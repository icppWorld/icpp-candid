// Binding decoded Candid values to the C++ variables of the typed API.
//
// CandidDeserialize decodes a message with CandidDecoder, at the types the
// CandidType* objects describe (graph_type), and gives each object its value
// (bind). For the classes that hold one C++ value, m_v, and an optional
// pointer to the caller's variable, m_pv, these helpers do the conversion:
//
//   bool, nat8 ... int64, float32/64   <- the value
//   nat, int                           <- the value; traps if it does not fit
//                                         in 128 bits
//   text                               <- the UTF-8 bytes
//   principal                          <- its text, e.g. "aaaaa-aa"
//   null, reserved                     <- "NULL_VALUE", "RESERVED_VALUE"
//   std::vector<T>                     <- a vec
//   std::optional<T>                   <- an opt

#pragma once

#include <cstring>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "candid_principal_text.h"
#include "candid_value.h"
#include "icpp_hooks.h"

namespace candid_bind {

inline void assign(const CandidValue &v, bool &out) { out = v.b; }

template <typename T>
inline std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>
assign(const CandidValue &v, T &out) {
  if constexpr (std::is_signed_v<T>) out = static_cast<T>(v.i);
  else out = static_cast<T>(v.u);
}

inline void assign(const CandidValue &v, float &out) {
  out = static_cast<float>(v.f64);
}
inline void assign(const CandidValue &v, double &out) { out = v.f64; }

inline void assign(const CandidValue &v, __uint128_t &out) {
  if (!v.n.to_u128(out))
    ICPP_HOOKS::trap("ERROR: a nat on the wire does not fit in 128 bits");
}

inline void assign(const CandidValue &v, __int128_t &out) {
  if (!v.n.to_i128(out))
    ICPP_HOOKS::trap("ERROR: an int on the wire does not fit in 128 bits");
}

inline void assign(const CandidValue &v, std::string &out) {
  switch (v.kind) {
  case CandidKind::Principal:
    out = CandidPrincipalText::encode(v.bytes);
    break;
  case CandidKind::Null:
    out = "NULL_VALUE";
    break;
  case CandidKind::Reserved:
    out = "RESERVED_VALUE";
    break;
  default:
    out = v.text;
  }
}

template <typename T> void assign(const CandidValue &v, std::vector<T> &out);
template <typename T> void assign(const CandidValue &v, std::optional<T> &out);

template <typename T> void assign(const CandidValue &v, std::vector<T> &out) {
  size_t n = v.vec_size();
  out.clear();
  if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
    // Elements of a fixed width arrive packed, little-endian, as in memory
    size_t width = 0;
    bool is_float = v.elem_kind == CandidKind::Float32 ||
                    v.elem_kind == CandidKind::Float64;
    if (v.packed && CandidTypeGraph::is_fixed_width(v.elem_kind, width) &&
        width == sizeof(T) && is_float == std::is_floating_point_v<T>) {
      out.resize(n);
      if (n) std::memcpy(out.data(), v.bytes.data(), n * sizeof(T));
      return;
    }
  }
  out.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    T x{};
    assign(v.packed ? v.packed_element(i) : v.items[i], x);
    out.push_back(std::move(x));
  }
}

template <typename T> void assign(const CandidValue &v, std::optional<T> &out) {
  if (v.items.empty()) {
    out = std::nullopt;
    return;
  }
  T x{};
  assign(v.items[0], x);
  out = std::move(x);
}

// Appends one element to the vec bound by m_pv, else to m_v
template <typename T>
void push_back(const CandidValue &e, std::vector<T> &v, std::vector<T> *pv) {
  T x{};
  assign(e, x);
  if (pv) pv->push_back(std::move(x));
  else v.push_back(std::move(x));
}

} // namespace candid_bind

// The bind methods of a class with members m_v and m_pv
#define CANDID_BIND_VALUE                                                      \
  void bind(const CandidValue &v) {                                            \
    candid_bind::assign(v, m_v);                                               \
    if (m_pv) *m_pv = m_v;                                                     \
  }

// ... and of a vec class, whose elements CandidTypeVecRecord appends one by one
#define CANDID_BIND_VEC                                                        \
  CANDID_BIND_VALUE                                                            \
  void bind_push_back(const CandidValue &e) {                                  \
    candid_bind::push_back(e, m_v, m_pv);                                      \
  }
