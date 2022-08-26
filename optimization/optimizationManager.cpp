#include "optimization.h"

void optimize(Program* prog){

  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    for(auto iter_bbs = fn->bbs.begin(); iter_bbs != fn->bbs.end(); ++iter_bbs){
      BasicBlock* bb = *iter_bbs;
      const bool optimized = optimize_bb(bb);
    } //for iter_bbs
  } //for fn
  
  return;
} //optimize()
