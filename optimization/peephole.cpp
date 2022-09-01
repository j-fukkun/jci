#include "optimization.h"

bool optimize_bb(BasicBlock* bb){
  //changed IR --> return true
  //otherwise --> return false
  bool changed = false;
  changed = changed || peephole(bb);
  changed = changed || constantPropagation_bb(bb);
  changed = changed || DCE_bb(bb);
  
  return changed;
} //optimize_bb()

bool isBinaryOp(const IRKind opcode){
  return opcode == IR_ADD
    || opcode == IR_SUB
    || opcode == IR_MUL
    || opcode == IR_DIV
    || opcode == IR_MOD
    || opcode == IR_EQ
    || opcode == IR_NE
    || opcode == IR_LT
    || opcode == IR_LE
    || opcode == IR_PTR_ADD
    || opcode == IR_PTR_SUB
    || opcode == IR_PTR_DIFF
    || opcode == IR_SHL
    || opcode == IR_SHR
    || opcode == IR_BITOR
    || opcode == IR_BITAND
    || opcode == IR_BITXOR;
} //isBinaryOp()

bool isUnaryOp(const IRKind opcode){
  return opcode == IR_MOV
    //|| opcode == IR_RETURN
    || opcode == IR_STORE
    || opcode == IR_LOAD
    || opcode == IR_BR;
} //isUnaryOp()

bool constantPropagation_bb(BasicBlock* bb){

  bool changed = false;
  using cons = std::pair<Reg*, int>;
  std::unordered_map<Reg*, int> table = {};
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;

    //generation
    if(ir->opcode == IR_IMM){
      table.insert(cons{ir->d, ir->imm});
    } //if IR_IMM

    if(ir->opcode == IR_MOV && ir->b->isImm){
      table.insert(cons{ir->d, ir->b->imm});
    } //if IR_MOV

    //kill
    if(ir->opcode == IR_CAST){
      table.erase(ir->a);
    }

    //replace
    if(isBinaryOp(ir->opcode)
       && ir->opcode != IR_DIV && ir->opcode != IR_MOD){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
	changed = true;
      }
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	ir->b->isImm = true;
	ir->b->imm = table.find(ir->b)->second;
	changed = true;
      }
    } //if bin-op

    //unary
    if(isUnaryOp(ir->opcode)){
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	ir->b->isImm = true;
	ir->b->imm = table.find(ir->b)->second;
	changed = true;
      }
    } //if unary-ops
    
    if(ir->opcode == IR_RETURN){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
	changed = true;
      }
    } //if RETURN

    if(ir->opcode == IR_JMP){
      if(ir->bbarg){
	if(table.find(ir->bbarg) != table.end() && !ir->bbarg->isImm){
	  ir->bbarg->isImm = true;
	  ir->bbarg->imm = table.find(ir->bbarg)->second;
	  changed = true;
	}
      }
    } //if JMP

    //function call
    if(ir->opcode == IR_FUNCALL){
      for(int i = 0; i < ir->num_args; i++){
	if(table.find(ir->args[i]) != table.end()
	   && !ir->args[i]->isImm){
	  ir->args[i]->isImm = true;
	  ir->args[i]->imm = table.find(ir->args[i])->second;
	  changed = true;
	}
      } //for
    }
    
  } //for iter_inst
  return changed;
} //constantPropagation_bb()

bool DCE_bb(BasicBlock* bb){

  bool changed = false;
  std::unordered_set<Reg*> table = {};

  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    
    if(ir->opcode != IR_STORE
       && ir->opcode != IR_STORE_SPILL
       && ir->opcode != IR_STORE_ARG
       && ir->opcode != IR_RETURN
       && ir->opcode != IR_BR
       && ir->opcode != IR_JMP
       && ir->opcode != IR_JMP_LABEL
       && ir->opcode != IR_LABEL
       && ir->opcode != IR_FUNCALL
       && ir->opcode != IR_CAST
       && ir->opcode != IR_LVAR
       ){
      table.insert(ir->d);
    } //if

    //binary
    if(isBinaryOp(ir->opcode)){
      if(ir->opcode == IR_DIV || ir->opcode == IR_MOD){
	if(table.find(ir->a) != table.end()){
	  table.erase(ir->a);
	}
	if(table.find(ir->b) != table.end()){
	  table.erase(ir->b);
	}
      } else {
	if(table.find(ir->a) != table.end() && !ir->a->isImm){
	  table.erase(ir->a);
	}
	if(table.find(ir->b) != table.end() && !ir->b->isImm){
	  table.erase(ir->b);
	}
      }
    } //if binary

    //unary
    if(isUnaryOp(ir->opcode)){
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	table.erase(ir->b);
      }
    } //if unary

    if(ir->opcode == IR_STORE
       || ir->opcode == IR_STORE_SPILL
       || ir->opcode == IR_RETURN
       || ir->opcode == IR_CAST){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	table.erase(ir->a);
      }
    }
    
    //fuction call
    if(ir->opcode == IR_FUNCALL){
      for(int i = 0; i < ir->num_args; i++){
	if(table.find(ir->args[i]) != table.end()
	   && !ir->args[i]->isImm){
	  table.erase(ir->args[i]);
	}
      } //for
    } //if funcall
    
  } //for iter_inst

  
  //IRs in table are dead code
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end();){
    IR* ir = *iter_inst;
    if(ir->opcode != IR_STORE
       && ir->opcode != IR_STORE_SPILL
       && ir->opcode != IR_RETURN
       && ir->opcode != IR_BR
       && ir->opcode != IR_JMP
       && ir->opcode != IR_JMP_LABEL
       && ir->opcode != IR_LABEL
       && ir->opcode != IR_FUNCALL
       && ir->opcode != IR_CAST
       ){      
      if(table.find(ir->d) != table.end()){
	iter_inst = bb->instructions.erase(iter_inst);
	changed = true;
	continue;
      }      
    } //if
    ++iter_inst;
  } //for iter_inst

  return changed;
} //DCE_bb()

static IR* createMove(Reg* d, Reg* b, const int imm){
  IR* ir = new IR();
  ir->opcode = IR_MOV;
  ir->d = d;
  ir->b = b;
  ir->b->isImm = true;
  ir->b->imm = imm;
  return ir;
} //createMove()

bool peephole(BasicBlock* bb){

  bool changed = false;
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(isBinaryOp(ir->opcode)
       && ir->a->isImm && ir->b->isImm){      
	if(ir->opcode == IR_ADD){
	  const int c = ir->a->imm + ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //ADD
	else if(ir->opcode == IR_SUB){
	  const int c = ir->a->imm - ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //SUB
	else if(ir->opcode == IR_MUL){
	  const int c = ir->a->imm * ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //MUL
	else if(ir->opcode == IR_DIV){
	  const int c = ir->a->imm / ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //DIV
	else if(ir->opcode == IR_MOD){
	  const int c = ir->a->imm % ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //MOD
	else if(ir->opcode == IR_EQ){
	  const int c = ir->a->imm == ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //EQ
	else if(ir->opcode == IR_NE){
	  const int c = ir->a->imm != ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //NE
	else if(ir->opcode == IR_LT){
	  const int c = ir->a->imm < ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //LT
	else if(ir->opcode == IR_LE){
	  const int c = ir->a->imm <= ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //LE
	else if(ir->opcode == IR_SHL){
	  const int c = ir->a->imm << ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //SHL
	else if(ir->opcode == IR_SHR){
	  const int c = ir->a->imm >> ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //SHR
	else if(ir->opcode == IR_BITOR){
	  const int c = ir->a->imm | ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //BITOR
	else if(ir->opcode == IR_BITAND){
	  const int c = ir->a->imm & ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //BITAND
	else if(ir->opcode == IR_BITXOR){
	  const int c = ir->a->imm ^ ir->b->imm;
	  IR* new_ir = createMove(ir->d, ir->b, c);	  
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	} //BITXOR
    } //if
  } //for iter_inst
  return changed;
} //peephole()

