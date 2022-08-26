#include "optimization.h"

bool optimize_bb(BasicBlock* bb){
  //changed IR --> return true
  //otherwise --> return false
  bool changed = false;
  
  changed = changed || constantPropagation_bb(bb);
  
  return changed;
} //optimize_bb()

bool constantPropagation_bb(BasicBlock* bb){

  using cons = std::pair<Reg*, int>;
  std::unordered_map<Reg*, int> table = {};
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(ir->opcode == IR_IMM){
      table.insert(cons{ir->d, ir->imm});
    } //if IR_IMM

    if(ir->opcode == IR_ADD
       || ir->opcode == IR_SUB
       || ir->opcode == IR_MUL
       || ir->opcode == IR_DIV
       || ir->opcode == IR_MOD
       || ir->opcode == IR_EQ
       || ir->opcode == IR_NE
       || ir->opcode == IR_LT
       || ir->opcode == IR_LE
       || ir->opcode == IR_PTR_ADD
       || ir->opcode == IR_PTR_SUB
       || ir->opcode == IR_SHL
       || ir->opcode == IR_SHR
       || ir->opcode == IR_BITOR
       || ir->opcode == IR_BITAND
       || ir->opcode == IR_BITXOR
       ){
      if(table.find(ir->a) != table.end()){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
      } else if(table.find(ir->b) != table.end()){
	ir->b->isImm = true;
	ir->b->imm = table.find(ir->b)->second;
      }
    } //if bin-op

    //uni-op

    //function call
    
  } //for iter_inst
  
} //constantPropagation_bb()
