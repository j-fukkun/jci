#include "jcc.h"

static int nreg = 1;
static Function* func = nullptr;
static BasicBlock* out = nullptr;

BasicBlock* new_bb(){
  //BasicBlock* bb = (BasicBlock*)calloc(1, sizeof(BasicBlock));
  BasicBlock* bb = new BasicBlock();
  bb->label = nlabel++;
  func->bbs.push_back(bb);
  return bb;  
} //new_bb()

IR* new_ir(const IRKind opcode){
  IR* ir = new IR();
  ir->opcode = opcode;
  out->instructions.push_back(ir);
  return ir;
} //new_ir()

Reg* new_reg(){
  Reg* reg = new Reg();
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

IR* br(Reg* r, BasicBlock* then, BasicBlock* els){
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

IR* jmp_arg(BasicBlock* bb, Reg* r){
  IR* ir = new_ir(IR_JMP);
  ir->bb1 = bb;
  //ir->bbarg = r;
  return ir;
} //jmp_arg()

void load(Node* node, Reg* dst, Reg* src){
  IR *ir = emit_IR(IR_LOAD, dst, NULL, src);
  ir->type_size = node->type->size;
} //load()

Reg* gen_lval_IR(Node* node){
  
  if(node->kind == ND_DEREF){
    return gen_expr_IR(node->lhs);
  }
  
  assert(node->kind == ND_VAR);
  Var* var = node->var;
  IR* ir;
  if(var->is_local){
    ir = new_ir(IR_LVAR);
    ir->d = new_reg();
    ir->lvar = var;
  } else {
    ir = new_ir(IR_LABEL_ADDR);
    ir->d = new_reg();
    ir->name = var->name;
  }
  return ir->d;
} //gen_lval_IR()

Reg* gen_binop_IR(const IRKind op, Node* node){
  Reg* d = new_reg();
  Reg* a = gen_expr_IR(node->lhs);
  Reg* b = gen_expr_IR(node->rhs);
  IR* ir = emit_IR(op, d, a, b);
  ir->type_base_size = (node->type->base != nullptr) ? node->type->base->size : 0;
  return d;
} //gen_binop_IR()

Reg* gen_expr_IR(Node* node){

  switch(node->kind){
  case ND_NULL:
    return nullptr;
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
  case ND_PTR_ADD:
    return gen_binop_IR(IR_PTR_ADD, node);
  case ND_PTR_SUB:
    return gen_binop_IR(IR_PTR_SUB, node);
  case ND_PTR_DIFF:
    return gen_binop_IR(IR_PTR_DIFF, node);
  case ND_EQ:
    return gen_binop_IR(IR_EQ, node);
  case ND_NE:
    return gen_binop_IR(IR_NE, node);
  case ND_LT:
    return gen_binop_IR(IR_LT, node);
  case ND_LE:
    return gen_binop_IR(IR_LE, node);
  case ND_VAR: {
    //配列はアドレスを計算するだけで良い
    if(node->type->kind != TY_ARRAY){
      Reg* r = new_reg();
      load(node, r, gen_lval_IR(node));
      return r;
    }
    return gen_lval_IR(node);
  } //ND_LVAR
  case ND_ASSIGN: {
    Reg* d = gen_lval_IR(node->lhs);
    Reg* a = gen_expr_IR(node->rhs);
    IR* ir = emit_IR(IR_STORE, NULL, d, a);
    //IR* ir = emit_IR(IR_STORE, d, a, NULL);
    ir->type_size = node->type->size;
    return a;
  } //ND_ASSIGN
  case ND_RETURN: {
    Reg* r = gen_expr_IR(node->lhs);
    IR* ir = new_ir(IR_RETURN);
    ir->a = r;
    ir->d = nullptr;
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
    Reg* args[6];
    int num_args = 0;
    Node* arg = node->args;
    for(arg; arg; arg = arg->next){
      args[num_args] = gen_expr_IR(arg);
      num_args++;
    } //for
    
    IR* ir = new_ir(IR_FUNCALL);
    ir->d = new_reg();
    ir->funcname = node->funcname;
    ir->num_args = num_args;
    memcpy(ir->args, args, sizeof(args));
    return ir->d;
  } //ND_FUNCALL
  case ND_ADDR:
    return gen_lval_IR(node->lhs);
  case ND_DEREF: {
    //配列はloadしない
    if(node->type->kind != TY_ARRAY){
      Reg* r = new_reg();
      load(node, r, gen_expr_IR(node->lhs));
      return r;
    }
    return gen_expr_IR(node->lhs);
  } //ND_DEREF
    
  } //switch
} //gen_expr_IR

void gen_param(Var* param, const unsigned int i){
  IR* ir = new_ir(IR_STORE_ARG);
  ir->lvar = param;
  ir->imm = i;
  ir->type_size = param->type->size;
  
} //gen_param()

void gen_IR(Program* prog){

  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    func = fn;
    // Add an empty entry BB to make later analysis easy.
    out = new_bb();
    //BasicBlock* bb = new_bb();
    //jmp(bb);
    //out = bb;

    unsigned int i = 0;
    //Var* param = fn->params;
    std::list<Var*> params = fn->params;
    //for(param; param; param = param->next, i++){
    for(auto param = params.begin(), end = params.end();
	param != end; ++param, ++i){
      gen_param(*param, i);
    } //for param
    
    Node* n = fn->node;
    for(n; n; n = n->next){
      gen_expr_IR(n);
    } //for n
    
  } //for fn
  
} //gen_IR()

