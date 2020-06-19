#include "jcc.h"

LVar* locals;
int nlabel = 1;

LVar* find_lvar(Token* tok){
  LVar* var = locals;
  for(var; var; var = var->next){
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
      return var;
    } //if
  } //for
  return NULL;
} //find_lvar()

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

// program = function*
Program* program(){
  
  Function head = {};
  Function* curr = &head;

  while(!at_eof()){
    Function* fn = function();
    if(!fn){
      continue;
    }
    curr->next = fn;
    curr = curr->next;
    continue;
  } //while()

  Program* prog = (Program*)calloc(1, sizeof(Program));
  prog->fns = head.next;
  return prog;
} //program()

LVar* read_func_param(){

  Token* tok = consume_ident();
  if(tok){
    LVar* lvar = (LVar*)calloc(1, sizeof(LVar));

    lvar->next = locals;
    //locals.push_back(lvar);
    lvar->name = tok->str;
    lvar->len = tok->len;

    if(locals){
      lvar->offset = locals->offset + 8;
    } else {
      lvar->offset = 8;
    } //if

    locals = lvar;
    //locals.push_back(lvar);
    return lvar;
  } //if(tok)

  return NULL;
} //read_func_param()


void read_func_params(Function* fn){

  if(consume(")")){
    //引数なしのとき
    return;
  }

  fn->params = read_func_param();
  LVar* curr = fn->params;

  while(!consume(")")){
    expect(",");

    curr->next = read_func_param();
    curr = curr->next;    
  } //while

} //read_func_params()

//function = ident "(" params? ")" "{" stmt* "}"
//params = param ("," param)*
//param = ident
Function* function(){

  locals = NULL;
  //locals.clear();

  char* name = expect_ident();
  //Function* fn = (Function*)calloc(1, sizeof(Function));
  Function* fn = new Function();
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
  fn->locals = locals; //segv
  
  return fn;  
} //function()

//stmt = expr ";"
//      | "{" stmt* "}"
//      | "return" expr? ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node* stmt(){
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

  //expr ";"
  node = expr();
  expect(";");
  return node;
  
  
} //stmt()

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

//add = mul ("+" mul | "-" mul)*
Node* add(){
  Node* node = mul();

  for(;;){
    if(consume(std::string("+").c_str())){
      node = new_binary(ND_ADD, node, mul());
    } else if(consume(std::string("-").c_str())){
      node = new_binary(ND_SUB, node, mul());
    } else {
      return node;
    } //if
  } //for
  
} //add()


// mul = unary ("*" unary | "/" unary)*
Node* mul(){
  Node* node = unary();

  for (;;) {
    if (consume(std::string("*").c_str()))
      node = new_binary(ND_MUL, node, unary());
    else if (consume(std::string("/").c_str()))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
} //mul()

//unary = ("+" | "-" | "*" | "&")? unary
//        | primary
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
  return primary();

} //urary()

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

// primary = "(" expr ")"
//           | num
//           | ident func_args?
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
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar* lvar = find_lvar(tok);
    if(lvar){
      node->lvar = lvar;
      node->offset = lvar->offset;
    } else {
      lvar = (LVar*)calloc(1, sizeof(LVar));
      lvar->next = locals;
      //locals.push_back(lvar);
      lvar->name = tok->str;
      lvar->len = tok->len;
      
      if(locals){
	lvar->offset = locals->offset + 8;
      } else {
	lvar->offset = 8;
      } //if
      node->offset = lvar->offset;
      node->lvar = lvar;
      locals = lvar;
    } //if
    return node;
  } //if tok

  return new_num(expect_number());
} //primary()

