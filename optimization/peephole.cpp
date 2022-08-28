#include "optimization.h"

bool optimize_bb(BasicBlock* bb){
  //changed IR --> return true
  //otherwise --> return false
  bool changed = false;
  
  changed = changed || constantPropagation_bb(bb);
  
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
} //isBInaryOp()

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
