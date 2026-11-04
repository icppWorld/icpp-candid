// The Candid subtype relation t1 <: t2, between two type graphs.

#include "candid_subtype.h"

#include <algorithm>

bool CandidSubtype::is_subtype(const CandidTypeGraph &g1, int t1,
                               const CandidTypeGraph &g2, int t2) {
  m_overflow = false;
  bool r = sub(g1, t1, g2, t2);
  if (m_overflow) {
    // A check that hit the depth limit proves nothing: drop what it assumed
    for (const Key &k : m_log)
      m_assumed.erase(k);
    m_log.clear();
    return false;
  }
  return r;
}

bool CandidSubtype::sub(const CandidTypeGraph &g1, int t1,
                        const CandidTypeGraph &g2, int t2) {
  if (&g1 == &g2 && t1 == t2) return true;
  if (m_overflow) return false;
  if (m_depth >= m_max_depth) {
    m_overflow = true;
    return false;
  }
  ++m_depth;

  bool refs = CandidTypeGraph::is_node(t1) || CandidTypeGraph::is_node(t2);
  bool r;
  if (!refs) {
    r = sub_(g1, t1, g2, t2);
  } else {
    Key key{&g1, t1, &g2, t2};
    if (m_refuted.count(key)) {
      r = false;
    } else if (!m_assumed.insert(key).second) {
      r = true; // already assumed: coinduction
    } else {
      size_t mark = m_log.size();
      m_log.push_back(key);
      bool overflow_before = m_overflow;
      r = sub_(g1, t1, g2, t2);
      if (!r) {
        for (size_t i = mark; i < m_log.size(); ++i)
          m_assumed.erase(m_log[i]);
        m_log.resize(mark);
        if (m_overflow == overflow_before) m_refuted.insert(key);
      }
    }
  }
  --m_depth;
  return r;
}

bool CandidSubtype::tuple_sub(const CandidTypeGraph &g1,
                              const std::vector<int> &xs,
                              const CandidTypeGraph &g2,
                              const std::vector<int> &ys) {
  // As records with fields 0..n-1: every field of the supertype must be in
  // the subtype, or be null, opt or reserved
  for (size_t i = 0; i < ys.size(); ++i) {
    if (i < xs.size()) {
      if (!sub(g1, xs[i], g2, ys[i])) return false;
    } else if (!g2.is_null_like(ys[i])) {
      return false;
    }
  }
  return true;
}

static const CandidGraphField *
find_field(const std::vector<CandidGraphField> &fs, uint32_t id) {
  auto it = std::lower_bound(
      fs.begin(), fs.end(), id,
      [](const CandidGraphField &f, uint32_t v) { return f.id < v; });
  if (it != fs.end() && it->id == id) return &*it;
  return nullptr;
}

static const CandidGraphMethod *
find_method(const std::vector<CandidGraphMethod> &ms, const std::string &name) {
  for (const auto &m : ms)
    if (m.name == name) return &m;
  return nullptr;
}

bool CandidSubtype::sub_(const CandidTypeGraph &g1, int t1,
                         const CandidTypeGraph &g2, int t2) {
  CandidKind k1 = g1.kind(t1);
  CandidKind k2 = g2.kind(t2);

  if (k2 == CandidKind::Reserved) return true;
  if (k1 == CandidKind::Empty) return true;
  if (k1 == CandidKind::Nat && k2 == CandidKind::Int) return true;
  if (k1 == CandidKind::Service && k2 == CandidKind::Principal) return true;
  if (k1 == CandidKind::Null && k2 == CandidKind::Opt) return true;

  // Anything is a subtype of an option: by the opt rules, or else by the
  // special rule that keeps subtyping transitive (a value that does not
  // coerce becomes null)
  if (k2 == CandidKind::Opt) return true;

  if (k1 != k2) return false;

  switch (k1) {
  case CandidKind::Vec:
    return sub(g1, g1.node(t1).inner, g2, g2.node(t2).inner);

  case CandidKind::Record: {
    const auto &fs1 = g1.node(t1).fields;
    for (const auto &f2 : g2.node(t2).fields) {
      const CandidGraphField *f1 = find_field(fs1, f2.id);
      if (f1) {
        if (!sub(g1, f1->type, g2, f2.type)) return false;
      } else if (!g2.is_null_like(f2.type) && !f2.optional) {
        return false;
      }
    }
    return true;
  }

  case CandidKind::Variant: {
    const auto &fs2 = g2.node(t2).fields;
    for (const auto &f1 : g1.node(t1).fields) {
      const CandidGraphField *f2 = find_field(fs2, f1.id);
      if (!f2 || !sub(g1, f1.type, g2, f2->type)) return false;
    }
    return true;
  }

  case CandidKind::Service: {
    const auto &ms1 = g1.node(t1).methods;
    for (const auto &m2 : g2.node(t2).methods) {
      const CandidGraphMethod *m1 = find_method(ms1, m2.name);
      if (!m1 || !sub(g1, m1->type, g2, m2.type)) return false;
    }
    return true;
  }

  case CandidKind::Func: {
    const CandidGraphNode &f1 = g1.node(t1);
    const CandidGraphNode &f2 = g2.node(t2);
    if (f1.modes != f2.modes) return false;
    // Arguments are contravariant, results covariant
    return tuple_sub(g2, f2.args, g1, f1.args) &&
           tuple_sub(g1, f1.rets, g2, f2.rets);
  }

  case CandidKind::Opt: // handled above
  default:
    // The same primitive type, or two future types
    return true;
  }
}
