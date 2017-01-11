#include "CheckIrreducibleCFG.h"

#include <set>
using std::set;

#include "llvm/PassSupport.h"
using llvm::RegisterPass;

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

set<BasicBlock*> intervalWithHeader(Graph<BasicBlock> cfg, BasicBlock *h) {
  set<BasicBlock *> result;
  result.insert(h);
  bool new_members;
  do {
    for (auto *ih : result) {
      new_members = false;
      for (auto *n : cfg.getSuccessors(ih)) {
        // n is already in I(h).
        if (result.find(n) != result.end()) continue;
        for (auto *ih : cfg.getPredecessors(n)) {
          // There is a predecessor not in I(h).
          if (result.find(ih) == result.end()) goto continue_outer;
        }
        new_members = true;
        result.insert(n);
    continue_outer:;
      }
    }
  } while (new_members);
  return result;
}

bool CheckIrreducibleCFGPass::runOnFunction(Function &F) {
  Graph<BasicBlock> cfg = construct_cfg(F);
  for (auto h : cfg.getNodes()) {
    auto ih = intervalWithHeader(cfg, h.first);
    outs() << "[DEBUG] Header size: " << ih.size() << '\n';
  }
  return false;
}
}
