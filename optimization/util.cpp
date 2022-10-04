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

static void print(FILE* file, const IR* ir){
  
  switch(ir->opcode){
  case IR_ADD:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = %d + %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = %d + v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = v%d + %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = v%d + v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_SUB:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = %d - %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = %d - v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = v%d - %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = v%d - v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_MUL:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = %d * %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = %d * v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = v%d * %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = v%d * v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_DIV:    
    fprintf(file, "v%d = v%d / v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);    
    break;
  case IR_MOD:    
    fprintf(file, "v%d = v%d % v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    break;
  case IR_IMM:
    fprintf(file, "IMM: v%d = %d\\l", ir->d->vn, ir->imm);
    break;
  case IR_MOV:
    if(ir->b->isImm){
      fprintf(file, "MOV: v%d = %d\\l", ir->d->vn, ir->b->imm);
    } else {
      fprintf(file, "MOV: v%d = v%d\\l", ir->d->vn, ir->b->vn);
    }
    break;
  case IR_EQ:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = %d == %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = %d == v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = v%d == %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = v%d == v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_NE:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = %d != %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = %d != v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = v%d != %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = v%d != v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_LT:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = LT %d, %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = LT %d, v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = LT v%d, %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = LT v%d, v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_LE:
    if(ir->a->isImm && ir->b->isImm){
      fprintf(file, "v%d = LE %d, %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
    } else if(ir->a->isImm){
      fprintf(file, "v%d = LE %d, v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
    } else if(ir->b->isImm){
      fprintf(file, "v%d = LE v%d, %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "v%d = LE v%d, v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
    }
    break;
  case IR_LVAR:
    fprintf(file, "Load_LVAR v%d [rbp-%d]\\l", ir->d->vn, ir->lvar->offset);
    break;
  case IR_STORE:
    if(ir->b->isImm){
      fprintf(file, "Store [v%d] %d\\l", ir->a->vn, ir->b->imm);
    } else {
      fprintf(file, "Store [v%d] v%d\\l", ir->a->vn, ir->b->vn);
    }
    break;
  case IR_STORE_SPILL:
    fprintf(file, "Store_Spill [rbp-%d] v%d\\l", ir->lvar->offset, ir->a->vn);
    break;
  case IR_LOAD:
    fprintf(file, "Load v%d [v%d]\\l", ir->d->vn, ir->b->vn);
    break;
  case IR_LOAD_SPILL:
    fprintf(file, "Load_Spill v%d [rbp-%d]\\l", ir->d->vn, ir->lvar->offset);
    break;
  case IR_RETURN:
	  if(ir->a != nullptr){
	    if(ir->a->isImm){
	      fprintf(file, "RETURN %d\\l", ir->a->imm);
	    } else {
	      fprintf(file, "RETURN v%d\\l", ir->a->vn);
	    }
	  } else {
	    fprintf(file, "RETURN\\l");
	  }
	  break;
	case IR_BR:
	  if(ir->b->isImm){
	    fprintf(file, "br %d, BB_%d, BB_%d\\l", ir->b->imm, ir->bb1->label, ir->bb2->label);
	  } else {
	    fprintf(file, "br v%d, BB_%d, BB_%d\\l", ir->b->vn, ir->bb1->label, ir->bb2->label);
	  }
	  break;
	case IR_JMP:
	  if(ir->bbarg){
	    if(ir->bbarg->isImm){
	      fprintf(file, "BBARG: v%d = %d\\l", ir->bb1->param->vn, ir->bbarg->imm);
	    } else {
	      fprintf(file, "BBARG: v%d = v%d\\l", ir->bb1->param->vn, ir->bbarg->vn);
	    }
	  }
	  fprintf(file, "jmp BB_%d\\l", ir->bb1->label);
	  break;
	case IR_JMP_LABEL:
	  fprintf(file, "jmp L.%s\\l", ir->dst_label);
	  break;
	case IR_LABEL:
	  fprintf(file, "L.%s\\l", ir->label);
	  break;
	case IR_FUNCALL:
	  fprintf(file, "v%d = call %s(", ir->d->vn, ir->funcname);
	  if(ir->num_args != 0){
	    if(ir->args[0]->isImm){
	      fprintf(file, "%d", ir->args[0]->imm);
	    } else {
	      fprintf(file, "v%d", ir->args[0]->vn);
	    }
	  }
	  for(int i = 1; i < ir->num_args; i++){
	    if(ir->args[i]->isImm){
	      fprintf(file, ", %d", ir->args[i]->imm);
	    } else {
	      fprintf(file, ", v%d", ir->args[i]->vn);
	    }
	  }
	  fprintf(file, ")\\l");
	  break;
	case IR_PTR_ADD:
	  if(ir->a->isImm){
	    fprintf(file, "PTR_ADD: v%d = %d + v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "PTR_ADD: v%d = v%d + %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "PTR_ADD: v%d = v%d + v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_PTR_SUB:
	  if(ir->a->isImm){
	    fprintf(file, "PTR_SUB: v%d = %d - v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "PTR_SUB: v%d = v%d - %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "PTR_SUB: v%d = v%d - v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_PTR_DIFF:
	  fprintf(file, "PTR_DIFF: v%d = v%d - v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  break;
	case IR_LABEL_ADDR:
	  fprintf(file, "Load_GLOBAL v%d, %s\\l", ir->d->vn, ir->name);
	  break;
	case IR_SHL:
	  if(ir->a->isImm && ir->b->isImm){
	    fprintf(file, "v%d = shl %d, %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
	  } else if(ir->a->isImm){
	    fprintf(file, "v%d = shl %d, v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "v%d = shl v%d, %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "v%d = shl v%d, v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_SHR:
	  if(ir->a->isImm && ir->b->isImm){
	    fprintf(file, "v%d = shr %d, %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
	  } else if(ir->a->isImm){
	    fprintf(file, "v%d = shr %d, v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "v%d = shr v%d, %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "v%d = shr v%d, v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_BITOR:
	  if(ir->a->isImm && ir->b->isImm){
	    fprintf(file, "v%d = %d | %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
	  } else if(ir->a->isImm){
	    fprintf(file, "v%d = %d | v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "v%d = v%d | %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "v%d = v%d | v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_BITAND:
	  if(ir->a->isImm && ir->b->isImm){
	    fprintf(file, "v%d = %d & %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
	  } else if(ir->a->isImm){
	    fprintf(file, "v%d = %d & v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "v%d = v%d & %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "v%d = v%d & v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_BITXOR:
	  if(ir->a->isImm && ir->b->isImm){
	    fprintf(file, "v%d = %d ^ %d\\l", ir->d->vn, ir->a->imm, ir->b->imm);
	  } else if(ir->a->isImm){
	    fprintf(file, "v%d = %d ^ v%d\\l", ir->d->vn, ir->a->imm, ir->b->vn);
	  } else if(ir->b->isImm){
	    fprintf(file, "v%d = v%d ^ %d\\l", ir->d->vn, ir->a->vn, ir->b->imm);
	  } else {
	    fprintf(file, "v%d = v%d ^ v%d\\l", ir->d->vn, ir->a->vn, ir->b->vn);
	  }
	  break;
	case IR_CAST:
	  fprintf(file, "CAST: v%d = v%d\\l", ir->a->vn, ir->a->vn);
	  break;
	default:
	  break;
	} //switch
} //print()


void printCFG(Function* fn){
  
  std::string dot = std::string(fn->name) + ".dot";
  FILE* f = fopen(dot.c_str(), "w");
  if(!f) std::cerr << "cannot open .dot file" << std::endl;

  fprintf(f, "digraph \"CFG for %s function\" {\n", fn->name);
  
  fprintf(f, "\tNode_START [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{start}\"];\n");
  fprintf(f, "\tNode_END [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{end}\"];\n");
  for(auto it_succ = fn->start_node->succ.begin(); it_succ != fn->start_node->succ.end(); ++it_succ){
    BasicBlock* s = *it_succ;
    fprintf(f, "\tNode_START -> Node_BB%d;\n", s->label);
  } //for it_succ
  
  for(auto iter_bbs = fn->bbs.begin(); iter_bbs != fn->bbs.end(); ++iter_bbs){
    BasicBlock* bb = *iter_bbs;
    fprintf(f, "\tNode_BB%d [shape=record, color=\"#3d50c3ff\", style=filled, fillcolor=\"#e36c5570\",label=\"{BB_%d:\\l", bb->label, bb->label);

    for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
      IR* ir = *iter_inst;
      print(f, ir);      
    } //for iter_inst
    fprintf(f, "}\"];\n");

    for(auto it_succ = bb->succ.begin(); it_succ != bb->succ.end(); ++it_succ){
      BasicBlock* s = *it_succ;
      if(s->isEndNode){
	fprintf(f, "\tNode_BB%d -> Node_END;\n", bb->label);
      } else {
	fprintf(f, "\tNode_BB%d -> Node_BB%d;\n", bb->label, s->label);
      }
    } //for it_succ
    
  } //for iter_bbs
  fprintf(f, "}");
  fclose(f);
} //printCFG()
