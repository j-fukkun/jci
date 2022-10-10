#include "optimization.h"

void optimize(Program* prog){

  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    for(auto iter_bbs = fn->bbs.begin(); iter_bbs != fn->bbs.end(); ++iter_bbs){
      BasicBlock* bb = *iter_bbs;
      bool optimized = optimize_bb(bb);
      while(optimized){
	optimized = optimize_bb(bb);
      }
    } //for iter_bbs
  } //for fn

  
  constructCFGs(prog);

  eliminateUnreachableBBs(prog);

  simplifyCFGs(prog);
  
  printCFGs(prog);
    
  return;
} //optimize()
