#include "optimization.h"

static void eliminateUnreachableBBs_Fn(Function* fn){
  //eliminate BBs that do not have any predecessors
  for(auto iter_bbs = fn->bbs.begin(); iter_bbs != fn->bbs.end(); ++iter_bbs){
    BasicBlock* bb = *iter_bbs;
    if(bb->pred.empty()){
      //This bb is unreachable
      //remove the edge bb --> bb's succ
      for(auto it_succ = bb->succ.begin(); it_succ != bb->succ.end(); ++it_succ){
	BasicBlock* succ_bb = *it_succ;
	for(auto it_pred = succ_bb->pred.begin(); it_pred != succ_bb->pred.end();
	    ++it_pred){
	  BasicBlock* p = *it_pred;
	  if(p->label == bb->label){
	    it_pred = succ_bb->pred.erase(it_pred);
	    it_pred = std::prev(it_pred);
	  } //if
	} //for it_pred
      } //for it_succ

      iter_bbs = fn->bbs.erase(iter_bbs);
      iter_bbs = std::prev(iter_bbs);
    } //if(bb->pred.empty())
    
  } //for iter_bbs
  
} //eliminateUnreachableBBs_Fn()

void eliminateUnreachableBBs(Program* prog){

  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    eliminateUnreachableBBs_Fn(fn);
  }
  return;
} //eliminateUnreachableBBs()
