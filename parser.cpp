#include "jcc.h"

Var* locals = NULL;
Var* globals = NULL;
int nlabel = 1;

Var* find_lvar(Token* tok){
  Var* var = locals;
  for(var; var; var = var->next){
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
      return var;
    } //if
  } //for
  return NULL;
} //find_lvar()

/*
Var* add_lvar(Type* type, char* name){
  Var* var = (Var*)calloc(1, sizeof(Var));
  var->type = type;
  var->is_local = true;
  var->name = name;
  lvar->next = locals;
  locals = lvar;
  return lvar;
} //add_lvar()
*/

Var* find_gvar(Token* tok){
  Var* var = globals;
  for(var; var; var = var->next){
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
      return var;
    } //if
  } //for
  return NULL;
} //find_gvar()

Node* new_node(const NodeKind kind){
  Node* node = (Node*)calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node* new_binary(const NodeKind kind, Node* lhs, Node* rhs){
  Node* node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_num(const int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = int_type; //とりあえず、int_typeにしておく
  return node;
}

Var* new_var(const char* name, Type* type, const bool is_local){
  Var* var = (Var*)calloc(1, sizeof(Var));
  var->name = name;
  var->len = strlen(name);
  var->type = type;
  var->is_local = is_local;
  return var;
} //new_var()

Node* new_var_node(Var* v){
  Node* node = new_node(ND_VAR);
  node->var = v;
  return node;
} //new_var_node()


//ローカル変数のnew
Var* new_lvar(const char* name, Type* type){
  Var* lvar = new_var(name, type, true);
  lvar->next = locals;
  locals = lvar;
  return lvar;
} //new_lvar

//グローバル変数のnew
Var* new_gvar(char* name, Type* type, const bool is_literal, char* literal){
  Var* gvar = new_var(name, type, false);
  gvar->next = globals;
  gvar->is_literal = is_literal;
  gvar->literal = literal;
  globals = gvar;
  return gvar;
} //new_gvar()

Node* new_expr(const NodeKind kind, Node* e){
  Node* node = new_node(kind);
  node->expr = e;
  return node;
} //new_expr()

Node* new_deref(Var* var){
  return new_unary(ND_DEREF, new_var_node(var));
} //new_deref()

Node* new_stmt_expr(/*const*/ std::vector<Node*>/*&*/ vec){
  Node* last = vec.back();
  vec.pop_back();
  
  std::vector<Node*> v;

  for(auto iter = vec.begin(), end = vec.end(); iter != end; ++iter){
    Node* n = new_expr(ND_EXPR_STMT, *iter);
    add_type(n);
    v.push_back(n);
  } //for

  Node* node = new_node(ND_STMT_EXPR);
  node->stmt = v;
  node->expr = last;
  add_type(node);
  return node;

} //new_stmt_expr()

Initializer* new_init_val(Initializer* cur, const int sz, const int val){
  Initializer* init = (Initializer*)calloc(1, sizeof(Initializer));
  init->size = sz;
  init->val = val;
  cur->next = init;
  return init;
} //new_init_val()

Initializer* new_init_zero(Initializer* cur, const int nbytes){
  //nbytes分、0埋め
  for(int i = 0; i < nbytes; i++){
    cur = new_init_val(cur, 1, 0);
  } //for
  return cur;
} //new_init_zero()

const bool consume_end() {
  Token *tok = token;
  if(consume("}") || (consume(",") && consume("}"))){
    return true;
  }
  token = tok;
  return false;
} //consume_end()

const bool peek_end() {
  Token *tok = token;
  const bool ret = consume("}") || (consume(",") && consume("}"));
  token = tok;
  return ret;
} //peek_end()

void expect_end() {
  if(!consume_end()){
    expect("}");
  }
} //expect_end()

void skip_excess_elements2() {
  for (;;) {
    if (consume("{")){
      skip_excess_elements2();
    }
    else {
      assign();
    }

    if (consume_end()){
      return;
    }
    expect(",");
  } //for

} //skip_excess_elements2()

void skip_excess_elements() {
  expect(",");
  warn_tok(token, "excess elements in initializer");
  skip_excess_elements2();
} //skip_excess_elements()


//basetype = builtin-type "*"*
//builtin-type = "int" | "char" | "void"
Type* basetype(){

  Type* type = nullptr;
  if(consume("int")){
    type = int_type;
  } else if(consume("char")){
    type = char_type;
  } else if(consume("void")){
    type = void_type;
  } //if
  
  while(consume("*")){
    type = pointer_to(type);
  }
  return type;
} //basetype()

//determine whether "global_var" or "function"
bool is_function(){
  Token* t = token;
  bool is_func = false;

  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();
  is_func = consume("(");

  token = t;
  return is_func;  
} //is_function()

//gvar_initializer2 = assign
//                    | "{" (gvar_initializer2 ("," gvar_initializer2)* ","? )? "}"
Initializer* gvar_initializer2(Initializer* cur, Type* type){

  Token* tok = token;

  if(type->kind == TY_ARRAY && type->base->kind == TY_CHAR
     && token->kind == TK_STR){
    //in case of char a[]="hoge";
    token = token->next;

    if(type->is_incomplete){
      //要素数が省略されているとき
      type->size = tok->str_len;
      type->array_size = tok->str_len;
      type->is_incomplete = false;
    } //if

    //compare array_size with string_length
    const int len = (type->array_size < tok->str_len)
      ? type->array_size : tok->str_len;

    for(int i = 0; i < len; i++){
      cur = new_init_val(cur, 1, tok->strings[i]);
    } //for
    //type->array_size >= tok->str_len のとき、その差を0埋めする
    return new_init_zero(cur, type->array_size - len);
  } //if TY_ARRAY && TY_CHAR && TK_STR
  

  if(type->kind == TY_ARRAY){
    //in case of TYPE a[] = {hoge, fuga};

    const bool open = consume("{");
    int i = 0;
    //要素数が省略されているとき、何要素でも許容する
    const int limit = type->is_incomplete ? INT_MAX : type->array_size;

    if(!peek("}")){
      do{
	cur = gvar_initializer2(cur, type->base);
	i++;
      } while(i < limit && !peek_end() && consume(","));
    } //if

    if(open && !consume_end()){
      //要素数が超過しているとき
      //T a[3] = {1,2,3,4,5}
      skip_excess_elements();
    } //if

    //set array elements which is not initialized to zero
    //T a[5] = {1,2,3}
    cur = new_init_zero(cur, type->base->size * (type->array_size - i));

    if(type->is_incomplete){
      //要素数が省略されているとき
      type->size = type->base->size * i;
      type->array_size = i;
      type->is_incomplete = false;
    } //if
    return cur;    
  } //if(type->kind == TY_ARRAY)

  
  //if(type->kind == TY_STRUCT)
  //未実装
  
  const bool open = consume("{");
  Node* expression = expr();
  if(open){
    expect_end();
  }

  const int constant = eval(expression);

  return new_init_val(cur, type->size, constant);
} //gvar_initializer2()

Initializer* gvar_initializer(Type *type) {
  Initializer head = {};
  gvar_initializer2(&head, type);
  return head.next;
}

//global_var = basetype ident type_suffix ("=" gvar_initializer)? ";"
void global_var(){
  
  Type* type = basetype();
  Token* tok = token;
  char* name = expect_ident();
  type = type_suffix(type);

  if(type->kind == TY_VOID){
    error_tok(tok, "variable declared void");
  }
 
  Var* gvar = new_gvar(name, type, false, NULL);

  if(consume("=")){
    gvar->initializer = gvar_initializer(type);
    expect(";");
    return;
  } //if

  if(type->is_incomplete){
    error_tok(tok, "incomplete type");
  }
  expect(";");
  
} //global_var()

// program = (global_var | function)*
Program* program(){
  
  Function head = {};
  Function* curr = &head;

  globals = NULL;

  while(!at_eof()){
    if(is_function()){
      Function* fn = function();
      if(!fn){
	continue;
      }
      curr->next = fn;
      curr = curr->next;
      continue;
    } //if
    global_var();
  } //while()

  Program* prog = (Program*)calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
} //program()


//param = basetype ident type_suffix
Var* read_func_param(){

  Type* type = basetype();
  Token* tok = consume_ident();
  type = type_suffix(type);

  if(type->kind == TY_ARRAY){
    type = pointer_to(type->base);
  } //if
  
  if(tok){
    char* name = strndup(tok->str, tok->len);
    return new_lvar(name, type);
  } //if(tok)

  return nullptr;
} //read_func_param()

//params = param ("," param)*
void read_func_params(Function* fn){

  if(consume(")")){
    //引数なしのとき
    return;
  }

  Token* tok = token;
  if(consume("void") && consume(")")){
    //args is void
    return;
  }
  token = tok;

  //fn->params = read_func_param();
  //Var* curr = fn->params;
  std::list<Var*> params;
  params.push_back(read_func_param());
  
  while(!consume(")")){
    expect(",");

    //curr->next = read_func_param();
    //curr = curr->next;
    params.push_back(read_func_param());
  } //while
  fn->params = params;

} //read_func_params()

//function = basetype ident "(" params? ")" "{" stmt* "}"
//params = param ("," param)*
//param = basetype ident type_suffix
Function* function(){

  locals = NULL;

  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();
  Function* fn = new Function();
  //fn->name = strndup(tok->str, tok->len);
  fn->name = name;
  expect("(");
  read_func_params(fn);


  //read function body
  Node head = {};
  Node* curr = &head;
  expect("{");
  while(!consume("}")){
    curr->next = stmt();
    curr = curr->next;
  } //while

  fn->node = head.next;
  fn->locals = locals; 
  return fn;  
} //function()


bool is_typename(){

  return peek("int") || peek("char") || peek("void");

} //is_typename()

Node* new_desg_node2(Var* var, Designator* desg) {
  if(!desg){
    return new_var_node(var);
  }

  Node* node = new_desg_node2(var, desg->next);
  /*
  if(desg->mem){
    node = new_unary(ND_MEMBER, node, desg->mem->tok);
    node->member = desg->mem;
    return node;
  }
  */

  node = new_add(node, new_num(desg->index));
  return new_unary(ND_DEREF, node);
} //new_desg_node2()

Node* new_desg_node(Var* var, Designator* desg, Node* rhs) {
  Node* lhs = new_desg_node2(var, desg);
  Node* node = new_binary(ND_ASSIGN, lhs, rhs);
  add_type(node);
  Node* ret = new_expr(ND_EXPR_STMT, node);
  add_type(ret);
  return ret;
} //new_desg_node()

Node* lvar_init_zero(Node* cur, Var* var, Type* ty, Designator* desg) {
  if (ty->kind == TY_ARRAY) {
    for (int i = 0; i < ty->array_size; i++) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, var, ty->base, &desg2);
    }
    return cur;
  }

  cur->next = new_desg_node(var, desg, new_num(0));
  return cur->next;
} //lvar_init_zero()

Node* lvar_initializer2(Node* cur, Var* lvar, Type* type, Designator* desg){

  Token* tok = token;
  
  if(type->kind == TY_ARRAY && type->base->kind == TY_CHAR
     && token->kind == TK_STR){
    //in case of char a[]="hoge";
    token = token->next;

    if(type->is_incomplete){
      type->size = tok->str_len;
      type->array_size = tok->str_len;
      type->is_incomplete = false;
    } //if

    //compare array_size with string_length
    int len = (type->array_size < tok->str_len)
      ? type->array_size : tok->str_len;

    int i = 0;
    for(i; i < len; i++){
      Designator desg2 = {desg, i};
      Node *rhs = new_num(tok->strings[i]);
      cur->next = new_desg_node(lvar, &desg2, rhs);
      cur = cur->next;
    } //for

    //initialize zero
    for (int i = len; i < type->array_size; i++) {
      Designator desg2 = {desg, i};
      cur = lvar_init_zero(cur, lvar, type->base, &desg2);
    }
    return cur;
  } //if TY_ARRAY && TY_CHAR && TK_STR

  if(type->kind == TY_ARRAY){
    bool open = consume("{");
    int i = 0;
    int limit = type->is_incomplete ? INT_MAX : type->array_size;

    if (!peek("}")) {
      do {
        Designator desg2 = {desg, i++};
        cur = lvar_initializer2(cur, lvar, type->base, &desg2);
      } while (i < limit && !peek_end() && consume(","));
    }

    if (open && !consume_end()){
      skip_excess_elements();
    }

    // Set array elements which is not initialized to zero.
    while (i < type->array_size) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, lvar, type->base, &desg2);
    }

    if (type->is_incomplete) {
      type->size = type->base->size * i;
      type->array_size = i;
      type->is_incomplete = false;
    }
    return cur;
  } //if(type->kind == TY_ARRAY)

  bool open = consume("{");
  cur->next = new_desg_node(lvar, desg, assign());
  if (open){
    expect_end();
  }
  return cur->next;
  
} //lvar_initializer2()

Node* lvar_initializer(Var* lvar){

  Node head = {};
  lvar_initializer2(&head, lvar, lvar->type, NULL);

  Node* node = new_node(ND_BLOCK);
  node->body = head.next;
  return node;
} //lvar_initializer()


//declaration = basetype ident type_suffix ("=" lvar_initializer)? ";"
Node* declaration(){
  Token* tok = token;
  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();

  type = type_suffix(type);

  if(type->kind == TY_VOID){
    error_tok(tok, "variable declared void");
  }

  Var* lvar = new_lvar(name, type);
  
  if(consume(";")){
    if(type->is_incomplete){
      error_tok(tok, "incomplete type");
    }
    return new_node(ND_NULL); //変数宣言では、コード生成はしない
  } //if

  expect("=");
  Node* node = lvar_initializer(lvar);
  expect(";");
  return node; 
  
} //declaration()


//type_suffix = ("[" const_expr "]" type_suffix)?
Type* type_suffix(Type* type){
  if(!consume("[")){
    return type;
  }

  int size = 0;
  bool is_incomplete = true; //index is omitted?

  if(!consume("]")){
    size = const_expr();
    is_incomplete = false;
    expect("]");
  } //if

  type = type_suffix(type);
  if(type->is_incomplete){
    error("incomplete type");
  }

  type = array_of(type, size);
  type->is_incomplete = is_incomplete;
  return type;
} //type_suffix()


const int const_expr(){
  return eval(add());
} //const_expr()


const int eval(Node* node){
  switch(node->kind){
  case ND_ADD:
    return eval(node->lhs) + eval(node->rhs);
  case ND_SUB:
    return eval(node->lhs) - eval(node->rhs);
  case ND_MUL:
    return eval(node->lhs) * eval(node->rhs);
  case ND_DIV:
    return eval(node->lhs) / eval(node->rhs);
  case ND_NUM:
    return node->val;

  } //switch()
  error("not a constant expression");
}


Node* stmt(){
  Node* node = stmt2();
  add_type(node);
  return node;
} //stmt()


//stmt2 = expr ";"
//      | "{" stmt* "}"
//      | "return" expr? ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | declaration
Node* stmt2(){
  Node* node;

  if(consume("return")){
    if(consume(";")){
      //"return" ";"
      node = new_node(ND_RETURN);
      return node;
    } //if
    // "return" expr ";"
    node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  } //if return

  if(consume("if")){
    //"if" "(" expr ")" stmt ("else" stmt)?
    node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if(consume("else")){
      node->els = stmt();
    }
    return node; 
  } //if if

  if(consume("while")){
    //"while" "(" expr ")" stmt
    node = new_node(ND_WHILE);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    return node;
  } //if "while"

  if(consume("for")){
    //"for" "(" expr? ";" expr? ";" expr? ")" stmt
    node = new_node(ND_FOR);
    expect("(");
    //1個目
    if(!consume(";")){
      //先読みして、";"ではなかったとき
      node->init = expr();
      expect(";");
    } //if(!consume(";"))

    //2個目
    if(!consume(";")){
      node->cond = expr();
      expect(";");
    } //if(!consume(";"))

    //3個目
    if(!consume(")")){
      node->inc = expr();
      expect(")");
    } //if(!consume(")"))
    node->then = stmt();
    return node;
  } //if "for"

  if(consume("{")){
    //"{" stmt* "}"
    Node head = {};
    Node* curr = &head;

    while(!consume("}")){
      curr->next = stmt();
      curr = curr->next;
    } //while

    node = new_node(ND_BLOCK);
    node->body = head.next;
    return node;    
  } //if(consume("{"))

  if(is_typename()){
    //変数宣言
    return declaration();
  } 

  //expr ";"
  node = expr();
  expect(";");
  return node;
  
} //stmt2()

// expr = assign
Node *expr(){
  return assign();
} //expr()


//a op= b; --> {T* t = &a; *t = *t op b;}
Node* new_assign_eq(NodeKind k, Node* lhs, Node* rhs){
  add_type(lhs);
  add_type(rhs);
  std::vector<Node*> v;

  //T* t = &a;
  Var* var = new_lvar("tmp", pointer_to(lhs->type));
  Node* node = new_binary(ND_ASSIGN, new_var_node(var), new_unary(ND_ADDR, lhs));
  add_type(node);
  v.push_back(node);
  
  //*t = *t op b;
  node = new_binary(ND_ASSIGN,
		    new_deref(var),
		    new_binary(k, new_deref(var), rhs)
		    );
  add_type(node);
  v.push_back(node);

  return new_stmt_expr(v);

} //new_assign_eq()


//assign = logor (assign-op assign)?
//assign-op = "=" | "+=" | "-=" | "*=" | "/=" 
Node* assign(){
  //Node* node = equality();
  Node* node = logor();
  if(consume("=")){
    return new_binary(ND_ASSIGN, node, assign());
  } //if

  if(consume("+=")){
    add_type(node);
    if(node->type->base){
      return new_assign_eq(ND_PTR_ADD, node, assign());
    } else {
      return new_assign_eq(ND_ADD, node, assign());
    }
  } //if +=

  if(consume("-=")){
    add_type(node);
    if(node->type->base){
      return new_assign_eq(ND_PTR_SUB, node, assign());
    } else {
      return new_assign_eq(ND_SUB, node, assign());
    }
  } //if -=

  if(consume("*=")){
    return new_assign_eq(ND_MUL, node, assign());
  }

  if(consume("/=")){
    return new_assign_eq(ND_DIV, node, assign());
  }
  
  return node;
} //assign()

//logor = logand ("||" logand)*
Node* logor(){
  Node* node = logand();
  while(consume("||")){
    node = new_binary(ND_LOGOR, node, logand());
  }  //while
  return node;
} //logor()

//logand = equality ("&&" equality)*
Node* logand(){
  Node* node = equality();
  while(consume("&&")){
    node = new_binary(ND_LOGAND, node, equality());
  }  //while
  return node;
} //logand()

//equality = relatinal ("==" relational | "!=" relational)*
Node* equality(){
  Node* node = relational();

  for(;;){
    if(consume(std::string("==").c_str())){
      node = new_binary(ND_EQ, node, relational());
    } else if(consume(std::string("!=").c_str())){
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    } //if
  } //for
} //equality()


//relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
Node* relational(){
  Node* node = shift();

  for(;;){
    if(consume("<")){
      node = new_binary(ND_LT, node, shift());
    } else if(consume("<=")){
      node = new_binary(ND_LE, node, shift());
    } else if(consume(">")){
      node = new_binary(ND_LT, shift(), node);
    } else if(consume(">=")){
      node = new_binary(ND_LE, shift(), node);
    } else {
      return node;
    }
  } //for
  
} //relational()

// shift = add ("<<" add | ">>" add)*
Node* shift(){
  Node* node = add();
  Token* tok;

  for(;;){
    if(consume("<<")){
      node = new_binary(ND_SHL, node, add());
    }
    else if(consume(">>")){
      node = new_binary(ND_SHR, node, add());
    }
    else{
      return node;
    }
  } //for
  
} //shift()

Node* new_add(Node* lhs, Node* rhs){
  add_type(lhs);
  add_type(rhs);

  if(is_integer(lhs->type) && is_integer(rhs->type)){
    return new_binary(ND_ADD, lhs, rhs);
  }
  if(lhs->type->base && is_integer(rhs->type)){
    //左辺がポインタ 右辺がint
    return new_binary(ND_PTR_ADD, lhs, rhs);
  }
  if(is_integer(lhs->type) && rhs->type->base){
    //左辺がint 右辺がポインタ
    return new_binary(ND_PTR_ADD, rhs, lhs);
  }
  error("invalid operands");

} //new_add()

Node* new_sub(Node* lhs, Node* rhs){
  add_type(lhs);
  add_type(rhs);

  if(is_integer(lhs->type) && is_integer(rhs->type)){
    return new_binary(ND_SUB, lhs, rhs);
  }
  if(lhs->type->base && is_integer(rhs->type)){
    //左辺がポインタ 右辺がint
    return new_binary(ND_PTR_SUB, lhs, rhs);
  }
  if(lhs->type->base && rhs->type->base){
    //左辺がポインタ 右辺がポインタ
    return new_binary(ND_PTR_DIFF, rhs, lhs);
  }
  error("invalid operands");

} //new_sub()

//add = mul ("+" mul | "-" mul)*
Node* add(){
  Node* node = mul();

  for(;;){
    if(consume("+")){
      //node = new_binary(ND_ADD, node, mul());
      node = new_add(node, mul());
    } else if(consume("-")){
      //node = new_binary(ND_SUB, node, mul());
      node = new_sub(node, mul());
    } else {
      return node;
    } //if
  } //for
  
} //add()


// mul = unary ("*" unary | "/" unary)*
Node* mul(){
  Node* node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
} //mul()

Node* new_unary(NodeKind kind, Node* lhs){
  Node* node = new_node(kind);
  node->lhs = lhs;
  return node;
} //new_unary()

//unary = ("+" | "-" | "*" | "&")? unary
//        | ("++" | "--") unary
//        | "sizeof" unary
//        | postfix
Node* unary(){
  if(consume("+")){
    //printf("unary() +\n");
    return unary();
  }
  if(consume("-")){
    return new_binary(ND_SUB, new_num(0), unary());
  }
  if(consume("*")){
    Node* node = new_node(ND_DEREF);
    node->lhs = unary();
    return node;
  }
  if(consume("&")){
    Node* node = new_node(ND_ADDR);
    node->lhs = unary();
    return node;
  }

  if(consume("++")){
    return new_unary(ND_PRE_INC, unary());
  }
  
  if(consume("--")){
    return new_unary(ND_PRE_DEC, unary());
  }
  
  if(consume("sizeof")){
    Node* node = unary();
    add_type(node);
    return new_num(node->type->size);
  }
  return postfix();

} //urary()


//i++ --> {T* t = &i; T t2 = *t; *t = *t + imm; t2;}
Node* new_post_inc(Node* n, const int imm){
  add_type(n);

  std::vector<Node*> vec;
  Var* t = new_lvar("tmp", pointer_to(n->type));
  Var* t2 = new_lvar("tmp2", n->type);

  //T* t = &i;
  vec.push_back(new_binary(ND_ASSIGN, new_var_node(t), new_unary(ND_ADDR, n)));

  //T t2 = *t;
  vec.push_back(new_binary(ND_ASSIGN, new_var_node(t2), new_deref(t)));
  
  //*t = *t + imm;
  vec.push_back(new_binary(ND_ASSIGN,
			   new_deref(t),
			   new_add(new_deref(t), new_num(imm))
			   )
		);

  //t2;
  vec.push_back(new_var_node(t2));
  return new_stmt_expr(vec);
  
} //new_post_inc()

//postfix = primary ("[" expr "]" | "++" | "--")*
Node* postfix(){

  Node* node = primary();

  for(;;){
    if(consume("[")){
      //a[b] => *(a + b)
      Node* tmp = new_add(node, expr());
      expect("]");
      node = new_unary(ND_DEREF, tmp);
      continue;
    } //if "["

    if(consume("++")){
      //node = new_unary(ND_POST_INC, node);
      node = new_post_inc(node, 1);
      continue;
    } //if ++

    if(consume("--")){
      //node = new_unary(ND_POST_DEC, node);
      node = new_post_inc(node, -1);
      continue;
    } //if --

    break;
  } //for

  return node;
} //postfix

Node* func_args(){

  if(consume(")")){
    //引数がないとき
    return nullptr;
  }

  Node* head = assign();
  Node* curr = head;
  while(consume(",")){
    curr->next = assign();
    curr = curr->next;
  } //while
  expect(")");
  return head;

} //func_args()

static char* new_label(){
  static int count = 0;
  char buf[20];
  sprintf(buf, ".L.data.%d", count);
  ++count;
  return strndup(buf, 20);
} //new_label()

// primary = "(" expr ")"
//           | ident func_args?
//           | string_literal
//           | num
Node* primary() {
  if(consume(std::string("(").c_str())) {
    Node *node = expr();
    expect(std::string(")").c_str());
    return node;
  } //if consume("(")

  Token* tok = consume_ident();
  if(tok){
    if(consume("(")){
      //function call
      Node* node = new_node(ND_FUNCALL);
      node->funcname = strndup(tok->str, tok->len); //文字列複製
      node->args = func_args();
      return node;
    } //if(consume("("))

    //variable
    Node* node = new_node(ND_VAR);
    Var* lvar = find_lvar(tok);
    Var* gvar = find_gvar(tok);

    if(lvar){
      node->var = lvar;
    } else if(gvar){
      node->var = gvar;
    } else {
      error_at(tok->str, "undefined variable");
    } //if
    return node;
  } //if tok

  tok = consume_str();
  if(tok){
    Type* type = array_of(char_type, tok->str_len);
    Var* gvar = new_gvar(new_label(), type, true, tok->strings);
    return new_var_node(gvar);
  } //if(tok) consume_str()

  return new_num(expect_number());
} //primary()

