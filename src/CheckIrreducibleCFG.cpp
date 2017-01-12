#include "CheckIrreducibleCFG.h"

#include <fstream>
using std::ofstream;

#include <set>
using std::set;

#include <list>
using std::list;

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
  cfg.beginNode = &*F.begin();
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
  vector<set<T *>*> sets;
  map<T *, set<T *>*> map;

  for (auto Node : cfg.getNodes()) {
    auto h = Node.first;
    outs() << "[DEBUG] Looking at node " << h << '\n';
    // h is already associated to a set.
    if (map.find(h) != map.end())
      continue;

    outs() << "[DEBUG] ... it is not in the map yet.\n";

    // Create a new set and get a reference to it.
    sets.push_back(new set<T *>);
    set<T *> &iwh = *sets.back();
    iwh.insert(h);
    map[h] = &iwh;
    bool new_members;
    do {
      for (auto *ih : iwh) {
        new_members = false;
        for (auto *n : cfg.getSuccessors(ih)) {
          // n is the begin node.
          if (n == cfg.beginNode)
            continue;
          // n is already in I(h).
          if (iwh.find(n) != iwh.end())
            continue;
          for (auto *ih : cfg.getPredecessors(n)) {
            // There is a predecessor not in I(h).
            if (iwh.find(ih) == iwh.end())
              goto continue_outer;
          }
          new_members = true;
          {
            auto it = map.find(n);
            if (it != map.end()) {
              outs() << "[DEBUG] Need to merge sets!!\n";
              for (auto wrong : map) {
                // Remap all nodes pointing to the old set.
                if (wrong.second == it->second) {
                  iwh.insert(wrong.first);
                  map[wrong.first] = &iwh;
                }
              }
            } else {
              // Mutual exclusion is not strictly necessary, but this is
              // conceptually clearer than adding n twice to the set etc.
              iwh.insert(n);
              map[n] = &iwh;
            }
          }
          outs() << "[DEBUG] Add node " << n << " to set of node " << h << '\n';
        continue_outer:;
        }
      }
    } while (new_members);
  }

  // Multiple nodes can be associated with the same set, so to avoid copying
  // sets, this map needs to be from T* to set<T*>*. Thus we need a separate
  // storage for the sets.
  IWHS<T> result;
  std::map<set<T *> *, unsigned> indeces;
  for (auto p : map) {
    auto it = indeces.find(p.second);
    if (it != indeces.end()) {
      result.map[p.first] = it->second;
      outs() << "[DEBUG] mapping " << p.first << " to " << it->second << '\n';
      continue;
    }
    indeces[p.second] = result.sets.size();
    result.map[p.first] = result.sets.size();
    result.sets.push_back(*p.second); // Make a copy.
  }

  for (auto *s : sets) {
    delete s;
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

  for (auto Node : cfg.getNodes()) {
    T *parent = Node.first;
    for (auto child : cfg.getSuccessors(parent)) {
      outs() << "[DEBUG] Looking at connection " << parent << " -> " << child << '\n';
      auto *p = &result.iwhs.sets[result.iwhs.map[parent]];
      auto *c = &result.iwhs.sets[result.iwhs.map[child]];
      outs() << "[DEBUG] p=" << p << " c=" << c << '\n';
      outs() << "[DEBUG] result.g.hasSuccessor(p, c): " << result.g.hasSuccessor(p, c) << '\n';
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

unsigned count = 1;
template <typename T> bool isReducible(const Graph<T> &g) {
  if (g.size() == 1)
    return true;
  auto dg = getDerivedGraph(g);
  dg.g.name = "F_" + std::to_string(count++);

  outs() << "[DEBUG] Derived Graph:";
  for (auto Node : dg.g.getNodes()) {
    auto *set = Node.first;
    outs() << ' ' << set->size();
  }
  outs() << '\n';

  string filename = dg.g.name + ".dot";
  outs() << "[DEBUG] writing file " << filename << '\n';
  ofstream out(filename);
  out << dg.g.toDot();
  out.close();

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

  cfg.name = "F_0";
  ofstream out(cfg.name + ".dot");
  out << cfg.toDot();
  out.close();

  if (!isReducible(cfg)) {
    errs() << "[CheckIrreducibleCFG] Function " << F.getName() << " in module "
           << F.getParent()->getName() << " has an irreducible CFG!\n";
  }

  return false;
}
}
