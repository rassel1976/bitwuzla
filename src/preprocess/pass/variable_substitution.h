#ifndef BZLA_PREPROCESS_PASS_VARIABLE_SUBSTITUTION_H_INCLUDED
#define BZLA_PREPROCESS_PASS_VARIABLE_SUBSTITUTION_H_INCLUDED

#include "backtrack/unordered_map.h"
#include "backtrack/vector.h"
#include "node/unordered_node_ref_map.h"
#include "preprocess/preprocessing_pass.h"
#include "util/statistics.h"

namespace bzla::preprocess::pass {

/**
 * Preprocessing pass to perform rewriting on all assertions.
 */
class PassVariableSubstitution : public PreprocessingPass
{
 public:
  PassVariableSubstitution(Env& env,
                           backtrack::BacktrackManager* backtrack_mgr);

  void apply(AssertionVector& assertions) override;

  /** Process term and apply currently cached substitutions. */
  Node process(const Node& term) override;

 private:
  void remove_indirect_cycles(std::unordered_map<Node, Node>& substs) const;

  bool is_direct_cycle(const Node& var, const Node& term) const;

  Node substitute(const Node& term,
                  const std::unordered_map<Node, Node>& substitutions,
                  std::unordered_map<Node, Node>& cache) const;

  /** Only required to check the current assertion level. */
  const backtrack::BacktrackManager* d_backtrack_mgr;
  /** Current set of variable substitutions. */
  backtrack::unordered_map<Node, Node> d_substitutions;
  /** Current set of variable substitution assertions. */
  backtrack::unordered_map<Node, std::pair<Node, Node>>
      d_substitution_assertions;

  /** Backtrackable cache. */
  class Cache : public backtrack::Backtrackable
  {
   public:
    Cache(backtrack::BacktrackManager* mgr);

    void push() override;

    void pop() override
    {
      // Nothing to do
    }

    /** @return Current substitution map. */
    std::unordered_map<Node, Node>& substitutions();

    /** @return Current substitution cache. */
    std::unordered_map<Node, Node>& cache();

   private:
    /** Backtrackable substitution map. One map per scope level. */
    backtrack::vector<std::unordered_map<Node, Node>> d_map;
    /** Backtrackable substitution cache. One cache per scope level. */
    backtrack::vector<std::unordered_map<Node, Node>> d_cache;
  };

  /** Backtrackable substitution cache. */
  Cache d_cache;

  struct Statistics
  {
    Statistics(util::Statistics& stats);
    util::TimerStatistic& time_apply;
    util::TimerStatistic& time_register;
    util::TimerStatistic& time_direct_cycle_check;
    util::TimerStatistic& time_remove_cycles;
    uint64_t& num_substs;
  } d_stats;
};

}  // namespace bzla::preprocess::pass
#endif
