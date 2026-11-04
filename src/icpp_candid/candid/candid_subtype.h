// The Candid subtype relation t1 <: t2, between two type graphs.
//
//     https://github.com/dfinity/candid/blob/master/spec/Candid.md#upgrading-and-subtyping
//
// The check is coinductive: a pair of types already under consideration is
// assumed to hold, which is what makes it terminate on recursive types. Pairs
// that failed are remembered, so a recursive type reached along many paths is
// not explored again and again. This follows dfinity/candid's subtype.rs.

#pragma once

#include "candid_type_graph.h"

#include <set>
#include <tuple>
#include <vector>

class CandidSubtype {
public:
  explicit CandidSubtype(size_t max_depth = 128) : m_max_depth(max_depth) {}

  bool is_subtype(const CandidTypeGraph &g1, int t1, const CandidTypeGraph &g2,
                  int t2);

private:
  using Key =
      std::tuple<const CandidTypeGraph *, int, const CandidTypeGraph *, int>;
  std::set<Key> m_assumed;
  std::vector<Key> m_log;
  std::set<Key> m_refuted;
  size_t m_max_depth;
  size_t m_depth{0};
  bool m_overflow{false};

  bool sub(const CandidTypeGraph &g1, int t1, const CandidTypeGraph &g2,
           int t2);
  bool sub_(const CandidTypeGraph &g1, int t1, const CandidTypeGraph &g2,
            int t2);
  bool tuple_sub(const CandidTypeGraph &g1, const std::vector<int> &xs,
                 const CandidTypeGraph &g2, const std::vector<int> &ys);
};
