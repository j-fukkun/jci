#include "jcc.h"

IR* new_ir(const IRKind opcode){
  IR* ir = (IR*)calloc(1, sizeof(IR));
  ir->opcode = opcode;
  IR_list.push_back(ir);
  return ir;
} //new_ir()

static int nreg = 1;
Reg* new_reg(){
  Reg* reg = (Reg*)calloc(1, sizeof(Reg));
  reg->vn = nreg++;
  reg->rn = -1;
  return reg;
} //new_reg()

Reg* new_imm(const int imm){
  Reg* reg = new_reg();
  IR* ir = new_ir(IR_IMM);
  ir->d = reg;
  ir->imm = imm;
  return reg;
} //new_imm()

IR* emit_IR(const IRKind op, Reg* d, Reg* a, Reg* b){
  IR* ir = new_ir(op);
  ir->d = d;
  ir->a = a;
  ir->b = b;
  return ir;
} //emit_IR()

Reg* gen_binop_IR(const IRKind op, Node* node){
  Reg* d = new_reg();
  Reg* a = gen_expr_IR(node->lhs);
  Reg* b = gen_expr_IR(node->rhs);
  emit_IR(op, d, a, b);
  return d;
} //gen_binop_IR()

Reg* gen_expr_IR(Node* node){

  switch(node->kind){
  case ND_NUM:
    return new_imm(node->val);
  case ND_ADD:
    return gen_binop_IR(IR_ADD, node);
  case ND_SUB:
    return gen_binop_IR(IR_SUB, node);
  case ND_MUL:
    return gen_binop_IR(IR_MUL, node);
  case ND_DIV:
    return gen_binop_IR(IR_DIV, node);
  case ND_EQ:
    return gen_binop_IR(IR_EQ, node);
  case ND_NE:
    return gen_binop_IR(IR_NE, node);
  case ND_LT:
    return gen_binop_IR(IR_LT, node);
  case ND_LE:
    return gen_binop_IR(IR_LE, node);
  } //switch
} //gen_expr_IR

