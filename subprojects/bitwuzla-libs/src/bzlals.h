#ifndef BZLALS__BZLALS_H
#define BZLALS__BZLALS_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bzlals {

class RNG;
struct BzlaLsMove;
class BitVector;
class BitVectorDomain;
class BitVectorNode;

namespace test {
class TestBzlaLs;
}

class BzlaLs
{
  friend class test::TestBzlaLs;

 public:
  using NodesIdTable = std::vector<std::unique_ptr<BitVectorNode>>;
  using ParentsSet   = std::unordered_set<uint32_t>;
  using ParentsMap   = std::unordered_map<uint32_t, ParentsSet>;

  enum Result
  {
    SAT,
    UNSAT,
    UNKNOWN,
  };

  enum OperatorKind
  {
    ADD,
    AND,
    ASHR,
    CONCAT,
    // DEC,
    EXTRACT,
    EQ,
    // IMPLIES,
    ITE,
    // INC,
    MUL,
    // NAND,
    // NE,
    // NEG,
    // NOR,
    NOT,
    // OR,
    // REDAND,
    // REDOR,
    // SDIV,
    SEXT,
    // SGT,
    // SGE,
    SHL,
    SHR,
    SLT,
    // SLE,
    // SREM,
    // SUB,
    UDIV,
    // UGT,
    // UGE,
    ULT,
    // ULE,
    UREM,
    // XNOR,
    XOR,
    // ZEXT,
  };

  BzlaLs(uint64_t max_nprops, uint32_t seed = 1234);

  uint32_t mk_node(uint32_t size);
  uint32_t mk_node(OperatorKind kind,
                   uint32_t size,
                   const std::vector<uint32_t>& children);
  uint32_t mk_indexed_node(OperatorKind kind,
                           uint32_t size,
                           uint32_t child0,
                           const std::vector<uint32_t>& indices);

  uint32_t mk_node(const BitVector& assignment, const BitVectorDomain& domain);
  uint32_t mk_node(OperatorKind kind,
                   const BitVectorDomain& domain,
                   const std::vector<uint32_t>& children);
  uint32_t mk_indexed_node(OperatorKind kind,
                           const BitVectorDomain& domain,
                           uint32_t child0,
                           const std::vector<uint32_t>& indices);

  const BitVector& get_assignment(uint32_t id) const;
  void set_assignment(uint32_t id, const BitVector& assignment);
  const BitVectorDomain& get_domain(uint32_t node) const;
  // void set_domain(uint32_t node, const BitVectorDomain& domain);

  void register_root(uint32_t root);

  uint32_t get_arity(uint32_t id) const;
  uint32_t get_child(uint32_t id, uint32_t idx) const;

 private:
  BitVectorNode* get_node(uint32_t id) const;
  bool is_leaf_node(const BitVectorNode* node) const;
  bool is_root_node(const BitVectorNode* node) const;
  void update_roots(uint32_t id);
  void update_cone(BitVectorNode* node, const BitVector& assignment);
  BzlaLsMove select_move(BitVectorNode* root, const BitVector& t_root);
  Result move();

  std::unique_ptr<RNG> d_rng;

  NodesIdTable d_nodes;
  std::unordered_set<uint32_t> d_roots;
  ParentsMap d_parents;

  std::unique_ptr<BitVector> d_one;

  uint64_t d_max_nprops;
  uint64_t d_nprops = 0;
  uint64_t d_nmoves = 0;
  uint32_t d_seed;
};

}  // namespace bzlals
#endif