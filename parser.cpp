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
  return node;
}

Var* new_var(char* name, Type* type, bool is_local){
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
}

//ローカル変数のnew
Var* new_lvar(char* name, Type* type){
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


//basetype = builtin-type "*"*
//builtin-type = "int" | "char"
Type* basetype(){

  Type* type = nullptr;
  if(consume("int")){
    type = int_type;
  } else if(consume("char")){
    type = char_type;
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

//global_var = basetype ident type_suffix ";"
void global_var(){
  
  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();
  type = type_suffix(type);
  expect(";");
  
  Var* gvar = new_gvar(/*tok*/name, type, false, NULL);
  
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


//param = basetype ident
Var* read_func_param(){

  Type* type = basetype();
  Token* tok = consume_ident();
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
//param = basetype ident
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

  return peek("int") || peek("char");

} //is_typename()


//declaration = basetype ident type_suffix ";"
Node* declaration(){

  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();

  type = type_suffix(type);
  
  expect(";");

  Var* lvar = new_lvar(/*tok*/name, type);
  return new_node(ND_NULL); //変数宣言では、コード生成はしない
    
} //declaration()


//type_suffix = ("[" const_expr "]" type_suffix)?
Type* type_suffix(Type* type){
  if(!consume("[")){
    return type;
  }

  int size = 0;

  if(!consume("]")){
    size = const_expr();

    expect("]");
  } //if

  type = type_suffix(type);

  type = array_of(type, size);

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

//assign = equality ("=" assign)?
Node* assign(){
  Node* node = equality();
  if(consume(std::string("=").c_str())){
    node = new_binary(ND_ASSIGN, node, assign());
  } //if
  return node;
} //assign()

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

//relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational(){
  Node* node = add();

  for(;;){
    if(consume(std::string("<").c_str())){
      node = new_binary(ND_LT, node, add());
    } else if(consume(std::string("<=").c_str())){
      node = new_binary(ND_LE, node, add());
    } else if(consume(std::string(">").c_str())){
      node = new_binary(ND_LT, add(), node);
    } else if(consume(std::string(">=").c_str())){
      node = new_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  } //for
  
} //relational()

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

Node* new_stmt_expr(const std::vector<Node*>& vec){
  Node* last = *(--vec.end());
  
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

//i++ --> t = i; i=i+1; t;
Node* new_post_inc(Node* n, const int imm){
  add_type(n);
  
  std::vector<Node*> vec;
  Var* t = new_lvar("tmp", n->type);
  vec.push_back(new_binary(ND_ASSIGN,
			   new_var_node(t),
			   n));
  vec.push_back(new_binary(ND_ASSIGN,
			   n,
			   new_add(n, new_num(imm))
			   )
		);
  Node* node_t = new_var_node(t);
  add_type(node_t);
  vec.push_back(node_t);
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

