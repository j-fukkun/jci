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
  ir->bbarg = r;
  return ir;
} //jmp_arg()

IR* jmp_label(char* label){
  IR* ir = new_ir(IR_JMP_LABEL);
  ir->dst_label = label;
  return ir;
} //jmp_label()

IR* label(char* label){
  IR* ir = new_ir(IR_LABEL);
  ir->label = label;
  return ir;
} //label()

void load(Node* node, Reg* dst, Reg* src){
  IR *ir = emit_IR(IR_LOAD, dst, NULL, src);
  ir->type_size = node->type->size;
} //load()

Reg* gen_lval_IR(Node* node){
  
  if(node->kind == ND_DEREF){
    return gen_expr_IR(node->lhs);
  } //if ND_DEREF

  if(node->kind == ND_MEMBER){
    Reg* r1 = new_reg();
    Reg* r2 = gen_lval_IR(node->lhs);
    Reg* r3 = new_imm(node->member->offset);
    emit_IR(IR_ADD, r1, r2, r3);
    return r1;
  } //if ND_MEMBER
  
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
  case ND_SHL:
    return gen_binop_IR(IR_SHL, node);
  case ND_SHR:
    return gen_binop_IR(IR_SHR, node);
  case ND_LOGOR: {
    BasicBlock* bb = new_bb();
    BasicBlock* set0 = new_bb();
    BasicBlock* set1 = new_bb();
    BasicBlock* last = new_bb();

    Reg* r1 = gen_expr_IR(node->lhs);
    br(r1, set1, bb);

    out = bb;
    Reg* r2 = gen_expr_IR(node->rhs);
    br(r2, set1, set0);

    out = set0;
    jmp_arg(last, new_imm(0));

    out = set1;
    jmp_arg(last, new_imm(1));

    out = last;
    out->param = new_reg();
    return out->param;
  } //ND_LOGOR
  case ND_LOGAND: {
    BasicBlock* bb = new_bb();
    BasicBlock* set0 = new_bb();
    BasicBlock* set1 = new_bb();
    BasicBlock* last = new_bb();

    br(gen_expr_IR(node->lhs), bb, set0);

    out = bb;
    br(gen_expr_IR(node->rhs), set1, set0);

    out = set0;
    jmp_arg(last, new_imm(0));

    out = set1;
    jmp_arg(last, new_imm(1));

    out = last;
    out->param = new_reg();
    return out->param;
  } //ND_LOGAND
  case ND_NOT: {
    Reg* d = new_reg();
    Reg* a = gen_expr_IR(node->lhs);
    emit_IR(IR_EQ, d, a, new_imm(0));
    return d;
  } //ND_NOT
  case ND_BITOR: {
    return gen_binop_IR(IR_BITOR, node);
  } //ND_BITOR
  case ND_BITAND: {
    return gen_binop_IR(IR_BITAND, node);
  } //ND_BITAND
  case ND_BITXOR: {
    return gen_binop_IR(IR_BITXOR, node);
  } //ND_BITXOR
  case ND_BITNOT: {
    Reg* d = new_reg();
    Reg* a = gen_expr_IR(node->lhs);
    emit_IR(IR_BITXOR, d, a, new_imm(-1));
    return d;
  } //ND_BITNOT
  case ND_PRE_INC:{
    //++i --> i = i + 1
    Node* n = new_node(ND_ASSIGN);
    n->lhs = node->lhs;
    n->rhs = new_add(node->lhs, new_num(1), token);
    add_type(n);
    return gen_expr_IR(n);
  } //ND_PRE_INC
  case ND_PRE_DEC:{
    //--i --> i = i - 1
    Node* n = new_node(ND_ASSIGN);
    n->lhs = node->lhs;
    n->rhs = new_sub(node->lhs, new_num(1), token);
    add_type(n);
    return gen_expr_IR(n);
  } //ND_PRE_DEC
    /*
  case ND_POST_INC:{
    //i++ --> t = i; i = i + 1; t;
    Node* n = new_node(ND_ASSIGN);
    
    
  } //ND_POST_INC
  case ND_POST_DEC:{

  } //ND_POST_DEC
    */
  case ND_VAR: {
    //配列はアドレスを計算するだけで良い
    if(node->type->kind != TY_ARRAY){
      Reg* r = new_reg();
      load(node, r, gen_lval_IR(node));
      return r;
    }
    return gen_lval_IR(node);
  } //ND_LVAR
  case ND_MEMBER: {
    Reg* r = new_reg();
    load(node, r, gen_lval_IR(node));
    return r;
  } //ND_MEMBER
  case ND_ASSIGN: {
    Reg* d = gen_lval_IR(node->lhs);
    Reg* a = gen_expr_IR(node->rhs);
    IR* ir = emit_IR(IR_STORE, NULL, d, a);
    //IR* ir = emit_IR(IR_STORE, d, a, NULL);
    ir->type_size = node->type->size;
    ir->type = node->type;
    return a;
    //return d;
  } //ND_ASSIGN
    
  case ND_STMT_EXPR: {
    for (auto iter = node->stmt.begin(), end = node->stmt.end(); iter != end; ++iter){
      //gen_stmt(node->stmts->data[i]);
      gen_expr_IR(*iter);
    }
    return gen_expr_IR(node->expr);
  } //ND_STMT_EXPR
    
  case ND_EXPR_STMT: {
    gen_expr_IR(node->expr);
    return nullptr;
  } //ND_EXPR_STMT
    
  case ND_RETURN: {
    Reg* r;
    if(node->lhs){
      r = gen_expr_IR(node->lhs);
    } else {
      r = nullptr;
    }
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
    node->_continue = new_bb();
    node->_break = new_bb();
    //BasicBlock* _break = new_bb();

    if(node->init){
      gen_expr_IR(node->init); //gen_stmt
    } //if
    jmp(cond);

    out = cond;
    if(node->cond){
      Reg* r = gen_expr_IR(node->cond);
      br(r, body, node->_break/*_break*/);
    } else {
      jmp(body);
    } //if

    out = body;
    gen_expr_IR(node->then); //gen_stmt
    jmp(node->_continue);

    out = node->_continue;
    if(node->inc){
      gen_expr_IR(node->inc);
    } //if
    jmp(cond);

    //out = _break;
    out = node->_break;
    return nullptr;
  } //ND_FOR
  case ND_WHILE: {
    BasicBlock* cond = new_bb();
    BasicBlock* body = new_bb();
    //BasicBlock* _break = new_bb();
    node->_continue = cond;
    node->_break = new_bb();

    out = cond;
    Reg* r = gen_expr_IR(node->cond);
    br(r, body, node->_break/*_break*/);

    out = body;
    gen_expr_IR(node->then); //gen_stmt
    jmp(cond);

    //out = _break;
    out = node->_break;
    return nullptr;
  } //ND_WHILE
  case ND_SWITCH: {
    node->_break = new_bb();
    node->_continue = new_bb();

    Reg* r = gen_expr_IR(node->cond);
    for(auto iter = node->cases.begin(), end = node->cases.end();
	iter != end; ++iter){
      Node* _case = *iter;
      _case->bb = new_bb();

      BasicBlock* next = new_bb();
      Reg* r2 = new_reg();
      emit_IR(IR_EQ, r2, r, new_imm(_case->val));
      br(r2, _case->bb, next);
      out = next;
    } //for iter

    if(node->_default){
      BasicBlock* bb_default = new_bb();
      node->_default->bb = bb_default;
      jmp(bb_default);
    } else {
      jmp(node->_break);
    } //if

    gen_expr_IR(node->body);
    jmp(node->_break);

    out = node->_break;
    return nullptr;
  } //ND_SWITCH
  case ND_CASE: {
    jmp(node->bb);
    out = node->bb;
    gen_expr_IR(node->body);      
    return nullptr;
  } //ND_CASE
  case ND_BREAK: {
    jmp(node->target->_break);
    //ここで新しくbbを作るとbbの作成順序的に
    //そのbbの命令は後でコード生成されるので，おかしくなる
    //out = new_bb(); 
    return nullptr; //break;
  } //ND_BREAK
  case ND_CONTINUE: {
    jmp(node->target->_continue);
    //out = new_bb();
    return nullptr;
  } //ND_CONTINUE
  case ND_GOTO: {
    jmp_label(node->label_name);
    return nullptr;
  } //ND_GOTO
  case ND_LABEL: {
    label(node->label_name);
    gen_expr_IR(node->lhs);
    return nullptr;
  } //ND_LABEL
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

//
//IR dump
//
void dump_IR(Program* prog){

  Function* fn= prog->fns;
  for(fn; fn; fn = fn->next){
    printf("%s:\n", fn->name);
    for(auto iter = fn->bbs.begin(), end = fn->bbs.end(); iter != end; ++iter){
      printf("BB_%d:\n", (*iter)->label);
      for(auto it_inst = (*iter)->instructions.begin(), end_inst = (*iter)->instructions.end(); it_inst != end_inst; ++it_inst){
	IR* ir = *it_inst;
	//const int d = ir->d->vn;
	//const int a = ir->a->vn;
	//const int b = ir->b->vn;
	
	switch(ir->opcode){
	case IR_IMM:
	  printf("v%d = %d\n", ir->d->vn, ir->imm);
	  break;
	case IR_MOV:
	  printf("v%d = v%d\n", ir->d->vn, ir->b->vn);
	  //printf("MOV\n");
	  break;
	case IR_ADD:
	  printf("v%d = v%d + v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	  //printf("ADD\n");
	  break;
	case IR_SUB:
	  printf("v%d = v%d - v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	  //printf("SUB\n");
	  break;
	case IR_MUL:
	  printf("v%d = v%d * v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	  //printf("MUL\n");
	  break;
	case IR_DIV:
	  printf("v%d = v%d / v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	  //printf("DIV\n");
	  break;
	case IR_LVAR:
	  printf("Load v%d [rbp-%d]\n", ir->d->vn, ir->lvar->offset);
	  break;
	case IR_STORE:
	  printf("Store [v%d] v%d\n", ir->a->vn, ir->b->vn);
	  break;
	case IR_LOAD:
	  printf("Load v%d [v%d]\n", ir->d->vn, ir->b->vn);
	  break;
	case IR_RETURN:
	  if(ir->a != nullptr){
	    printf("RETURN v%d\n", ir->a->vn);
	  } else {
	    printf("RETURN\n");
	  }
	  break;
	default:
	  break;
	} //switch
      } //for it_inst
    } //for iter
  } //for fn
  
} //dump_IR()

