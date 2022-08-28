#include "optimization.h"

bool optimize_bb(BasicBlock* bb){
  //changed IR --> return true
  //otherwise --> return false
  bool changed = false;
  changed = changed || peephole(bb);
  changed = changed || constantPropagation_bb(bb);
  changed = changed || peephole(bb);
  
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
    if(isBinaryOp(ir->opcode)){
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
    } //if unary
    
    if(ir->opcode == IR_RETURN){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
	changed = true;
      }
    }

    //function call
    if(ir->opcode == IR_FUNCALL){
      for (int i = 0; i < ir->num_args; i++){
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

bool peephole(BasicBlock* bb){

  bool changed = false;
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(isBinaryOp(ir->opcode)
       && ir->a->isImm && ir->b->isImm){
      //switch(ir->opcode){
      /*IR_ADD:*/if(ir->opcode == IR_ADD){	  
	  const int c = ir->a->imm + ir->b->imm;
	  IR* new_ir = new IR();
	  new_ir->opcode = IR_MOV;
	  new_ir->d = ir->d;
	  new_ir->b = ir->b;
	  new_ir->b->isImm = true;
	  new_ir->b->imm = c;
	  auto del_it = bb->instructions.erase(iter_inst);
	  auto new_it = bb->instructions.insert(del_it, new_ir);
	  iter_inst = new_it;
	  changed = true;
	  break;
	}
      //default:
      //break;
      //} //switch
    } //if
  } //for iter_inst
  return changed;
} //peephole()
