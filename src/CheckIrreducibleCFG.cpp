#include "CheckIrreducibleCFG.h"

#include <set>
using std::set;

#include <vector>
using std::vector;

#include "llvm/PassSupport.h"
using llvm::RegisterPass;

#include "llvm/IR/Module.h"
using llvm::Module;

#include "llvm/IR/BasicBlock.h"
using llvm::BasicBlock;

#include "llvm/IR/CFG.h"
using llvm::succ_begin;
using llvm::succ_end;

#include "llvm/Support/raw_os_ostream.h"
using llvm::outs;
using llvm::errs;

#include "Graph.h"
using cic::Graph;

namespace icsa {

char CheckIrreducibleCFGPass::ID = 0;
RegisterPass<CheckIrreducibleCFGPass>
    DecoupleLoopsRegister("cic", "Check for irreducible CFGs");

Graph<BasicBlock> construct_cfg(Function &F) {
  Graph<BasicBlock> cfg;
  for (auto I = F.begin(), E = F.end(); I != E; ++I) {
    cfg.addNode(&*I);
  }

  for (auto I = F.begin(), E = F.end(); I != E; ++I) {
    BasicBlock *parent = &*I;
    for (auto J = succ_begin(parent), F = succ_end(parent); J != F; ++J) {
      BasicBlock *child = *J;
      if (cfg.contains(child)) {
        cfg.addEdge(parent, child);
      } else {
        errs() << "[ERROR] CFG contains edges to blocks that are not in the "
                  "function!\n";
      }
    }
  }

  return cfg;
}

// If h is a node in cfg, the I(h) - interval with header h is the set s.t.:
// (1) h is in I(h);
// (2) If n is a node not yet in I(h), n is not the begin node, and all edges
//     entering n leave nodes in I(h), then add n to I(h).
// (3) Repeate step (2) until no more nodes can be added to I(h).
template <typename T> set<T *> intervalWithHeader(Graph<T> cfg, T *h) {
  set<T *> result;
  result.insert(h);
  bool new_members;
  do {
    for (auto *ih : result) {
      new_members = false;
      for (auto *n : cfg.getSuccessors(ih)) {
        // n is already in I(h).
        if (result.find(n) != result.end())
          continue;
        for (auto *ih : cfg.getPredecessors(n)) {
          // There is a predecessor not in I(h).
          if (result.find(ih) == result.end())
            goto continue_outer;
        }
        new_members = true;
        result.insert(n);
      continue_outer:;
      }
    }
  } while (new_members);
  return result;
}

template <typename T> struct IWHS {
  map<T *, unsigned> map;
  vector<set<T *>> sets;
};

// Computes the interval-with-header set for each node in the cfg and creates a
// map from nodes to iwhs. An extension to intervalWithHeader.
template <typename T> IWHS<T> getIntervalsWithHeaders(Graph<T> cfg) {
  // Multiple nodes can be associated with the same set, so to avoid copying
  // sets, this map needs to be from T* to set<T*>*. Thus we need a separate
  // storage for the sets.
  IWHS<T> result;

  for (auto Node : cfg.getNodes()) {
    auto h = Node.first;
    // h is already associated to a set.
    if (result.map.find(h) != result.map.end())
      continue;

    // Create a new set and get a reference to it.
    result.sets.push_back({});
    set<T *> &iwh = result.sets.back();
    iwh.insert(h);
    result.map[h] = result.sets.size() - 1;
    bool new_members;
    do {
      for (auto *ih : iwh) {
        new_members = false;
        for (auto *n : cfg.getSuccessors(ih)) {
          // n is already in I(h).
          if (iwh.find(n) != iwh.end())
            continue;
          for (auto *ih : cfg.getPredecessors(n)) {
            // There is a predecessor not in I(h).
            if (iwh.find(ih) == iwh.end())
              goto continue_outer;
          }
          new_members = true;
          iwh.insert(n);
          result.map[n] = result.sets.size() - 1;
        continue_outer:;
        }
      }
    } while (new_members);
  }
  return result;
}

// Also owns the memory where the nodes are stored (unlike the Graph class).
template <typename T> struct DerivedGraph {
  // Storage.
  IWHS<T> iwhs;
  Graph<set<T *>> g;
};

template <typename T> DerivedGraph<T> getDerivedGraph(Graph<T> cfg) {
  DerivedGraph<T> result;

  result.iwhs = getIntervalsWithHeaders(cfg);
  for (set<T *> &iwh : result.iwhs.sets) {
    result.g.addNode(&iwh);
  }

  for (auto &s : result.iwhs.sets) {
  }

  for (auto &p : result.iwhs.map) {
  }

  for (auto Node : cfg.getNodes()) {
    T *parent = Node.first;
    for (auto child : cfg.getSuccessors(parent)) {
      auto *p = &result.iwhs.sets[result.iwhs.map[parent]];
      auto *c = &result.iwhs.sets[result.iwhs.map[child]];
      if (p != c && !result.g.hasSuccessor(p, c)) {
        result.g.addEdge(p, c);
      }
    }
  }

  return result;
}

// Make a copy of the Graph, changing the ValueType to void.
template <typename T> Graph<void> eraseTypeInfo(Graph<T> g) {
  Graph<void> result;

  for (auto &Node : g.getNodes()) {
    result.addNode(Node.first);
  }

  for (auto &Node : g.getNodes()) {
    auto *parent = Node.first;
    for (auto *child : g.getSuccessors(parent)) {
      result.addEdge(parent, child);
    }
  }

  return result;
}

template <typename T> bool isReducible(const Graph<T> &g) {
  if (g.size() == 1)
    return true;
  auto dg = getDerivedGraph(g);

  if (dg.g.size() == 1)
    return true;
  // If all nodes in the derived graph are sets of single nodes, then the
  // derived graph is the limit of the original graph. Since it has more than
  // a single node, then the original graph is not reducible.
  bool isLimit = true;
  for (auto Node : dg.g.getNodes()) {
    auto *set = Node.first;
    isLimit &= set->size() == 1;
  }
  if (isLimit)
    return false;

  // It is necessary to erase the type info, so that infinite template expansion
  // is avoided.
  return isReducible(eraseTypeInfo(dg.g));
}

bool CheckIrreducibleCFGPass::runOnFunction(Function &F) {
  Graph<BasicBlock> cfg = construct_cfg(F);

  if (!isReducible(cfg)) {
    errs() << "[CheckIrreducibleCFG] Function " << F.getName() << " in module "
           << F.getParent()->getName() << " has an irreducible CFG!\n";
  }

  return false;
}
}
