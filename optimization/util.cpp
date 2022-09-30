#include "optimization.h"

void add_edges(BasicBlock* bb){
  
  if(bb->instructions.empty()) return;
  if(!bb->succ.empty()) return;

  IR* last = bb->instructions.back();

  if(last->bb1){
    bb->succ.push_back(last->bb1);
    last->bb1->pred.push_back(bb);
    add_edges(last->bb1);
  } //if(last->bb1)

  if(last->bb2){
    bb->succ.push_back(last->bb2);
    last->bb2->pred.push_back(bb);
    add_edges(last->bb2);
  } //if(last->bb2)
  
  return;

} //add_edges()

void constructCFG(Function* fn){
  //This function constructs the Control Flow Graph (CFG) of fn.
  //While constructing it,
  //the function adds the unique start node "s" and end node "e".
  //Every node of fn lies on a path from "s" to "e".
  //"s" has no predecessors, and "e" has no successors.

  BasicBlock* s = new BasicBlock();
  s->label = 0;
  s->isStartNode = true;
  s->isEndNode = false;

  BasicBlock* e = new BasicBlock();
  e->label = 0;
  e->isStartNode = false;
  e->isEndNode = true;

  fn->start_node = s;
  fn->end_node = e;

  //s --> the first basic block of fn
  BasicBlock* first = *(fn->bbs.begin());
  s->succ.push_back(first);

  //add edges of the body of fn
  add_edges(first);

  //the nodes that do not have any successor --> e
  for(auto iter_bbs = fn->bbs.begin(); iter_bbs != fn->bbs.end(); ++iter_bbs){
    BasicBlock* bb = *iter_bbs;
    if(bb->succ.empty()){
      bb->succ.push_back(e);
      e->pred.push_back(bb);
    } //if
  } //for iter_bbs
  
} //constructCFG()
