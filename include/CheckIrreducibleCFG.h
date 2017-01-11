#ifndef CIC_H
#define CIC_H

#include "llvm/IR/Function.h"
using llvm::Function;

#include "llvm/Pass.h"
using llvm::FunctionPass;
#include "llvm/PassAnalysisSupport.h"
using llvm::AnalysisUsage;

namespace icsa {

class CheckIrreducibleCFGPass : public FunctionPass {
public:
  static char ID;

  CheckIrreducibleCFGPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &Info) const override {
    Info.setPreservesAll();
  }
};
}

#endif
