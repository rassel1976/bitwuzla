#include "preprocess/pass/normalize.h"

#include <cmath>

#include "env.h"
#include "node/node_manager.h"
#include "node/node_ref_vector.h"
#include "node/node_utils.h"
#include "node/unordered_node_ref_set.h"

namespace bzla::preprocess::pass {

using namespace bzla::node;

/* --- PassNormalize public ------------------------------------------------- */

PassNormalize::PassNormalize(Env& env,
                             backtrack::BacktrackManager* backtrack_mgr)
    : PreprocessingPass(env, backtrack_mgr), d_stats(env.statistics())
{
}

std::unordered_map<Node, uint64_t>
PassNormalize::compute_factors(
    const Node& node, const std::unordered_map<Node, uint64_t>& parents)
{
  bool share_aware = !parents.empty();
  Kind kind = node.kind();
  std::unordered_map<Node, uint64_t> factors;

  // compute reference count as initial factors
  node_ref_vector visit{node};
  do
  {
    const Node& cur     = visit.back();
    auto [it, inserted] = factors.emplace(cur, 1);
    visit.pop_back();
    if (inserted)
    {
      if (cur.kind() == kind)
      {
        if (share_aware)
        {
          // treat as leaf if node of given kind has parent references
          // from outside the current 'kind' chain
          assert(d_parents.find(cur) != d_parents.end());
          assert(parents.find(cur) != parents.end());
          if (parents.at(cur) < d_parents.at(cur))
          {
            continue;
          }
        }
        visit.insert(visit.end(), cur.begin(), cur.end());
      }
    }
    else
    {
      it->second += 1;
    }
  } while (!visit.empty());
  // compute factors by pushing initial factors to leafs
  visit.push_back(node);
  unordered_node_ref_set cache;
  do
  {
    const Node& cur     = visit.back();
    auto [it, inserted] = cache.insert(cur);
    visit.pop_back();
    if (inserted && cur.kind() == kind)
    {
      if (share_aware)
      {
        // treat as leaf if node of given kind has parent references
        // from outside the current 'kind' chain
        assert(d_parents.find(cur) != d_parents.end());
        assert(parents.find(cur) != parents.end());
        if (parents.at(cur) < d_parents.at(cur))
        {
          continue;
        }
      }

      auto fit = factors.find(cur);
      assert(fit != factors.end());
      for (auto& child : cur)
      {
        assert(factors.find(child) != factors.end());
        factors[child] = fit->second + factors[child] - 1;
        if (cur.kind() == kind)
        {
          visit.push_back(child);
        }
      }
    }
  } while (!visit.empty());
  return factors;
}

namespace {
std::unordered_map<Node, uint64_t>
_count_parents(const node_ref_vector& nodes, Kind kind)
{
  std::unordered_map<Node, uint64_t> parents;
  node::unordered_node_ref_set cache;
  for (size_t i = 0, size = nodes.size(); i < size; ++i)
  {
    node::node_ref_vector visit{nodes[i]};
    parents.emplace(nodes[i], 1);
    do
    {
      const Node& cur     = visit.back();
      auto [it, inserted] = cache.insert(cur);
      visit.pop_back();
      if (inserted && cur.kind() == kind)
      {
        for (auto& child : cur)
        {
          parents[child] += 1;
          visit.push_back(child);
        }
      }
    } while (!visit.empty());
  }
  return parents;
}
}  // namespace

std::tuple<std::unordered_map<Node, uint64_t>,
           std::unordered_map<Node, uint64_t>,
           bool>
PassNormalize::get_normalized_factors(const Node& node0,
                                      const Node& node1,
                                      bool share_aware)
{
  assert(node0.kind() == node1.kind());
  bool normalized = false;
  Kind kind       = node0.kind();
  std::unordered_map<Node, uint64_t> parents =
      share_aware ? _count_parents({node0, node1}, kind)
                  : std::unordered_map<Node, uint64_t>();
  auto factors0 = compute_factors(node0, parents);
  auto factors1 = compute_factors(node1, parents);
  std::unordered_map<Node, uint64_t> res0, res1;
  // normalize common factors and record entries that are not in factors1
  for (const auto& f : factors0)
  {
    if (f.first.kind() == kind)
    {
      if (!share_aware) continue;
      // treat as leaf if node of given kind has parent references
      // from outside the 'kind' chain starting with node0, node1
      assert(d_parents.find(f.first) != d_parents.end());
      assert(parents.find(f.first) != parents.end());
      if (parents[f.first] == d_parents[f.first])
      {
        continue;
      }
    }
    auto fit = factors1.find(f.first);
    if (fit == factors1.end())
    {
      res0.insert(f);
      continue;
    }
    if (f.second == fit->second) continue;
    if (f.second > fit->second)
    {
      assert(res0.find(f.first) == res0.end());
      res0.emplace(f.first, f.second - fit->second);
      normalized = true;
    }
    else
    {
      assert(res1.find(f.first) == res1.end());
      res1.emplace(f.first, fit->second - f.second);
      normalized = true;
    }
  }
  // check factors1 for entries that are not in factors0
  for (const auto& f : factors1)
  {
    if (f.first.kind() == kind)
    {
      if (!share_aware) continue;
      // treat as leaf if node of given kind has parent references
      // from outside the 'kind' chain starting with node0, node1
      assert(d_parents.find(f.first) != d_parents.end());
      assert(parents.find(f.first) != parents.end());
      if (parents[f.first] == d_parents[f.first])
      {
        continue;
      }
    }
    auto fit = factors0.find(f.first);
    if (fit == factors0.end())
    {
      res1.insert(f);
    }
  }

  // factors on both sides cancelled out
  if (res0.empty() && res1.empty())
  {
    return {{}, {}, true};
  }

  return {res0, res1, normalized};
}

std::pair<Node, Node>
PassNormalize::_normalize_eq_mul(
    const std::unordered_map<Node, uint64_t>& factors0,
    const std::unordered_map<Node, uint64_t>& factors1,
    uint64_t bv_size)
{
  NodeManager& nm = NodeManager::get();
  std::vector<Node> lhs, rhs;
  if (factors0.empty())
  {
    lhs.push_back(nm.mk_value(BitVector::mk_one(bv_size)));
  }
  else
  {
    for (const auto& f : factors0)
    {
      assert(f.second);
      for (size_t i = 0, n = f.second; i < n; ++i)
      {
        lhs.push_back(f.first);
      }
    }
  }
  if (factors1.empty())
  {
    rhs.push_back(nm.mk_value(BitVector::mk_one(bv_size)));
  }
  else
  {
    for (const auto& f : factors1)
    {
      assert(f.second);
      for (size_t i = 0, n = f.second; i < n; ++i)
      {
        rhs.push_back(f.first);
      }
    }
  }
  assert(!lhs.empty() && !rhs.empty());

  std::sort(lhs.begin(), lhs.end(), [](const Node& a, const Node& b) {
    return a.id() < b.id();
  });
  std::sort(rhs.begin(), rhs.end(), [](const Node& a, const Node& b) {
    return a.id() < b.id();
  });

  Node left, right;
  size_t n = lhs.size();
  left     = lhs[n - 1];
  for (size_t i = 1; i < n; ++i)
  {
    left = nm.mk_node(Kind::BV_MUL, {lhs[n - i - 1], left});
  }
  n = rhs.size();
  right = rhs[n - 1];
  for (size_t i = 1; i < n; ++i)
  {
    right = nm.mk_node(Kind::BV_MUL, {rhs[n - i - 1], right});
  }
  return {left, right};
}

namespace {
bool
_normalize_factors_add(std::unordered_map<Node, uint64_t>& factors0,
                       std::unordered_map<Node, uint64_t>& factors1,
                       uint64_t bv_size)
{
  // Note: Factors must already be normalized in the sense that they only
  //       either appear on the left or right hand side (this function must
  //       be called with factors determined by get_normalized_factors()).

  // (a - b + c = -d + e) is normalized to (a + c + d = b + e)

  // ~x = ~(x + 1)
  // -x = ~x + 1

  NodeManager& nm = NodeManager::get();
  Node one        = nm.mk_value(BitVector::mk_one(bv_size));
  BitVector value = BitVector::mk_zero(bv_size);
  node_ref_vector erase;

  for (const auto& f : factors0)
  {
    const Node& cur = f.first;
    uint64_t factor = f.second;
    if (cur.is_value())
    {
      assert(factor);
      erase.push_back(cur);  // cache for deletion
      const BitVector& cur_val = cur.value<BitVector>();
      if (factor > 1)
      {
        BitVector fact = BitVector::from_ui(bv_size, factor, true);
        if (!fact.is_zero())
        {
          if (fact.is_one())
          {
            value.ibvadd(cur_val);
          }
          else
          {
            value.ibvadd(cur_val.bvmul(fact));
          }
        }
      }
      else
      {
        value.ibvadd(cur_val);
      }
    }
    else if (cur.is_inverted())
    {
      Node neg;
      if (cur[0].kind() == Kind::BV_ADD)
      {
        if (cur[0][0] == one)
        {
          erase.push_back(cur);  // cache for deletion
          neg = cur[0][1];
          value.ibvsub(BitVector::from_ui(bv_size, factor, true));
        }
        else if (cur[0][1] == one)
        {
          erase.push_back(cur);  // cache for deletion
          neg = cur[0][0];
          value.ibvsub(BitVector::from_ui(bv_size, factor, true));
        }
      }
      else
      {
        erase.push_back(cur);  // cache for deletion
        neg = cur[0];
      }
      if (!neg.is_null())
      {
        auto it = factors1.find(neg);
        if (it == factors1.end())
        {
          factors1.emplace(neg, factor);
        }
        else
        {
          it->second += factor;
        }
        value.ibvsub(BitVector::from_ui(bv_size, factor, true));
      }
    }
  }
  bool normalized = !erase.empty();
  for (auto& node : erase)
  {
    assert(factors0.find(node) != factors0.end());
    factors0.erase(node);
  }
  if (!value.is_zero())
  {
    Node val = nm.mk_value(value);
    assert(factors0.find(val) == factors0.end());
    factors0.emplace(nm.mk_value(value), 1);
  }

  return normalized;
}

Node
get_factorized_add(const Node& node, uint64_t factor)
{
  assert(factor);
  NodeManager& nm = NodeManager::get();
  uint64_t size   = node.type().bv_size();
  if (factor == 1)
  {
    return node;
  }
  BitVector fact = BitVector::from_ui(size, factor, true);
  if (fact.is_zero())
  {
    return nm.mk_value(fact);
  }
  if (fact.is_one())
  {
    return node;
  }
  return nm.mk_node(Kind::BV_MUL, {nm.mk_value(fact), node});
}
}  // namespace

std::pair<Node, Node>
PassNormalize::_normalize_eq_add(std::unordered_map<Node, uint64_t>& factors0,
                                 std::unordered_map<Node, uint64_t>& factors1,
                                 uint64_t bv_size)
{
  NodeManager& nm = NodeManager::get();

  size_t size0 = factors0.empty() ? 1 : factors0.size();
  size_t size1 = factors1.empty() ? 1 : factors1.size();
  std::vector<Node> lhs(size0), rhs(size1);
  if (factors0.empty())
  {
    lhs[0] = nm.mk_value(BitVector::mk_zero(bv_size));
  }
  else
  {
    size_t i = 0;
    for (const auto& f : factors0)
    {
      assert(f.second);
      lhs[i++] = f.first;
    }
  }
  if (factors1.empty())
  {
    rhs[0] = nm.mk_value(BitVector::mk_zero(bv_size));
  }
  else
  {
    size_t i = 0;
    for (const auto& f : factors1)
    {
      assert(f.second);
      rhs[i++] = f.first;
    }
  }
  assert(!lhs.empty() && !rhs.empty());

  std::sort(lhs.begin(), lhs.end(), [](const Node& a, const Node& b) {
    return a.id() < b.id();
  });
  std::sort(rhs.begin(), rhs.end(), [](const Node& a, const Node& b) {
    return a.id() < b.id();
  });

  BitVector lvalue = BitVector::mk_zero(bv_size);
  BitVector rvalue = BitVector::mk_zero(bv_size);
  Node left;
  for (size_t i = 0, size = lhs.size(); i < size; ++i)
  {
    Node l          = lhs[size - i - 1];
    auto it         = factors0.find(l);
    uint64_t factor = it == factors0.end() ? 1 : it->second;
    if (l.is_value())
    {
      lvalue.ibvadd(BitVector::from_ui(bv_size, factor, true)
                        .ibvmul(l.value<BitVector>()));
    }
    else
    {
      l    = get_factorized_add(l, factor);
      left = left.is_null() ? l : nm.mk_node(Kind::BV_ADD, {l, left});
    }
  }
  Node right;
  for (size_t i = 0, size = rhs.size(); i < size; ++i)
  {
    Node r          = rhs[size - i - 1];
    auto it         = factors1.find(r);
    uint64_t factor = it == factors1.end() ? 1 : it->second;
    if (r.is_value())
    {
      rvalue.ibvadd(BitVector::from_ui(bv_size, factor, true)
                        .ibvmul(r.value<BitVector>()));
    }
    else
    {
      r     = get_factorized_add(r, factor);
      right = right.is_null() ? r : nm.mk_node(Kind::BV_ADD, {r, right});
    }
  }
  // normalize values, e.g., (a + 2 = b + 3) -> (a - 1 = b)
  if (!lvalue.is_zero())
  {
    Node val;
    if (!rvalue.is_zero())
    {
      val = nm.mk_value(lvalue.ibvsub(rvalue));
      rvalue = BitVector::mk_zero(bv_size);
    }
    else
    {
      val = nm.mk_value(lvalue);
    }
    left = left.is_null() ? val : nm.mk_node(Kind::BV_ADD, {val, left});
  }
  else if (!rvalue.is_zero())
  {
    Node val = nm.mk_value(rvalue);
    right    = right.is_null() ? val : nm.mk_node(Kind::BV_ADD, {val, right});
  }
  if (left.is_null())
  {
    assert(lvalue.is_zero());
    left = nm.mk_value(lvalue);
  }
  if (right.is_null())
  {
    assert(rvalue.is_zero());
    right = nm.mk_value(rvalue);
  }
  return {left, right};
}

std::pair<Node, bool>
PassNormalize::normalize_eq_add_mul(const Node& node0,
                                    const Node& node1,
                                    bool share_aware)
{
  assert(node0.kind() == node1.kind());
  assert(node0.kind() == Kind::BV_MUL || node0.kind() == Kind::BV_ADD);

  NodeManager& nm = NodeManager::get();

  auto [factors0, factors1, normalized] =
      get_normalized_factors(node0, node1, share_aware);

  if (node0.kind() == Kind::BV_ADD)
  {
    uint64_t bv_size = node0.type().bv_size();
    if (_normalize_factors_add(factors0, factors1, bv_size) && !normalized)
    {
      normalized = true;
    }
    if (_normalize_factors_add(factors1, factors0, bv_size) && !normalized)
    {
      normalized = true;
    }
  }

  if (!normalized)
  {
    return {nm.mk_node(Kind::EQUAL, {node0, node1}), false};
  }

  if (factors0.empty() && factors1.empty())
  {
    return {nm.mk_value(true), true};
  }

  auto [left, right] =
      node0.kind() == Kind::BV_ADD
          ? _normalize_eq_add(factors0, factors1, node0.type().bv_size())
          : _normalize_eq_mul(factors0, factors1, node0.type().bv_size());

  return {nm.mk_node(Kind::EQUAL, {left, right}), true};
}

void
PassNormalize::apply(AssertionVector& assertions)
{
  util::Timer timer(d_stats.time_apply);

  d_cache.clear();
  assert(d_parents.empty());
  d_parents = count_parents(assertions);

  for (size_t i = 0, size = assertions.size(); i < size; ++i)
  {
    const Node& assertion = assertions[i];
    if (!processed(assertion))
    {
      const Node& processed = process(assertion);
      assertions.replace(i, processed);
      cache_assertion(processed);
    }
  }
  d_parents.clear();
  d_cache.clear();
}

Node
PassNormalize::process(const Node& node)
{
  bool share_aware = d_env.options().pp_normalize_share_aware();
  node_ref_vector visit{node};
  do
  {
    const Node& cur     = visit.back();
    auto [it, inserted] = d_cache.emplace(cur, Node());
    if (inserted)
    {
      visit.insert(visit.end(), cur.begin(), cur.end());
      continue;
    }
    else if (it->second.is_null())
    {
      std::vector<Node> children;
      for (const Node& child : cur)
      {
        auto itc = d_cache.find(child);
        assert(itc != d_cache.end());
        assert(!itc->second.is_null());
        children.push_back(itc->second);
      }
      if (cur.kind() == Kind::EQUAL && children[0].kind() == children[1].kind()
          && (children[0].kind() == Kind::BV_ADD
              || children[0].kind() == Kind::BV_MUL))
      {
        auto [res, normalized] =
            normalize_eq_add_mul(children[0], children[1], share_aware);
        it->second = res;
        if (normalized) d_stats.num_normalizations += 1;
      }
      else
      {
        it->second = node::utils::rebuild_node(cur, children);
      }
    }
    visit.pop_back();
  } while (!visit.empty());
  auto it = d_cache.find(node);
  assert(it != d_cache.end());
  return d_env.rewriter().rewrite(it->second);
}

/* --- PassEmbeddedConstraints private -------------------------------------- */

PassNormalize::Statistics::Statistics(util::Statistics& stats)
    : time_apply(stats.new_stat<util::TimerStatistic>(
        "preprocess::normalize::time_apply")),
      num_normalizations(
          stats.new_stat<uint64_t>("preprocess::normalize::num_normalizations"))
{
}

}  // namespace bzla::preprocess::pass
