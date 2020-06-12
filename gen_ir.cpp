#include "jcc.h"

static int nreg = 1;
static BasicBlock* out;

BasicBlock* new_bb(){
  //BasicBlock* bb = (BasicBlock*)calloc(1, sizeof(BasicBlock));
  BasicBlock* bb = new BasicBlock();
  bb->label = nlabel++;
  BB_list.push_back(bb);
  return bb;  
} //new_bb()

IR* new_ir(const IRKind opcode){
  IR* ir = (IR*)calloc(1, sizeof(IR));
  ir->opcode = opcode;
  //IR_list.push_back(ir);
  out->instructions.push_back(ir);
  return ir;
} //new_ir()

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

IR* br(Reg* r, BasicBlock* then, BasicBlock* els) {
  IR* ir = new_ir(IR_BR);
  ir->b = r;
  ir->bb1 = then;
  ir->bb2 = els;
  return ir;
} //br()

IR* jmp(BasicBlock* bb){
  IR* ir = new_ir(IR_JMP);
  ir->bb1 = bb;
  return ir;
} //jmp()

IR* jmp_arg(BasicBlock* bb, Reg* r) {
  IR* ir = new_ir(IR_JMP);
  ir->bb1 = bb;
  //ir->bbarg = r;
  return ir;
} //jmp_arg()

void load(Node* node, Reg* dst, Reg* src) {
  IR *ir = emit_IR(IR_LOAD, dst, NULL, src);
  //ir->size = node->ty->size;
} //load()

Reg* gen_lval_IR(Node* node){

  assert(node->kind == ND_LVAR);
  IR* ir;
  ir = new_ir(IR_LVAR);
  ir->d = new_reg();
  ir->lvar = node->lvar;
  return ir->d;
} //gen_lval_IR()

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
  case ND_LVAR: {
    Reg* r = new_reg();
    load(node, r, gen_lval_IR(node));
    return r;
  } //ND_LVAR
  case ND_ASSIGN: {
    Reg* d = gen_lval_IR(node->lhs);
    Reg* a = gen_expr_IR(node->rhs);
    IR* ir = emit_IR(IR_STORE, NULL, d, a);
    //ir->size = node->ty->size;
    return a;
  } //ND_ASSIGN
  case ND_RETURN: {
    Reg* r = gen_expr_IR(node->lhs);
    IR* ir = new_ir(IR_RETURN);
    ir->a = r;
    out = new_bb();
    return nullptr;
  } //ND_RETURN
  case ND_IF: {
    BasicBlock* then = new_bb();
    BasicBlock* els = new_bb();
    BasicBlock* last = new_bb();

    br(gen_expr_IR(node->cond), then, els);

    out = then;
    gen_expr_IR(node->then); //gen_stmt
    jmp(last);

    out = els;
    if(node->els){
      gen_expr_IR(node->els); //gen_stmt
    } //if
    jmp(last);

    out = last;
    return nullptr;    
  } //ND_IF
  case ND_FOR: {
    BasicBlock* cond = new_bb();
    BasicBlock* body = new_bb();
    //node->continue_ = new_bb();
    //node->break_ = new_bb();
    BasicBlock* _break = new_bb();

    if(node->init){
      gen_expr_IR(node->init); //gen_stmt
    } //if
    jmp(cond);

    out = cond;
    if(node->cond){
      Reg* r = gen_expr_IR(node->cond);
      br(r, body, /*node->break_*/_break);
    } else {
      jmp(body);
    } //if

    out = body;
    gen_expr_IR(node->then); //gen_stmt

    if(node->inc){
      gen_expr_IR(node->inc);
    } //if
    jmp(cond);

    out = _break;
    return nullptr;
  } //ND_FOR
  case ND_WHILE: {
    BasicBlock* cond = new_bb();
    BasicBlock* body = new_bb();
    BasicBlock* _break = new_bb();

    out = cond;
    Reg* r = gen_expr_IR(node->cond);
    br(r, body, _break);

    out = body;
    gen_expr_IR(node->then); //gen_stmt
    jmp(cond);

    out = _break;
    return nullptr;
  } //ND_WHILE
  case ND_BLOCK: {
    Node* n = node->body;
    while(n){
      gen_expr_IR(n);
      n = n->next;
    } //while
    return nullptr;
  } //ND_BLOCK
  case ND_FUNCALL: {
    //printf("test funcall\n");
    IR* ir = new_ir(IR_FUNCALL);
    ir->d = new_reg();
    //printf("test funcall2\n");
    ir->funcname = node->funcname;
    //printf("test funcall3\n");
    return ir->d;
  } //ND_FUNCALL
    
  } //switch
} //gen_expr_IR

void gen_IR(){

  // Add an empty entry BB to make later analysis easy.
  out = new_bb();
  //BasicBlock* bb = new_bb();
  //jmp(bb);
  //out = bb;
  
  int i = 0;
  for(i = 0; ir_code[i]; i++){
    gen_expr_IR(ir_code[i]);
  } //for
  
} //gen_IR()

