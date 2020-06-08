#include "jcc.h"

std::list<LVar*> locals;
int nlabel = 1;

LVar* find_lvar(Token* tok){
  //LVar* var = locals;
  for(auto iter = locals.begin(), end = locals.end(); iter != end; ++iter){
    LVar* var = *iter;
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

// program = stmt*
void program(){
  int i = 0;
  while(!at_eof()){
    ir_code[i++] = stmt();
  }
  ir_code[i] = NULL;
} //program()

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

//unary = ("+" | "-")? unary
//        | primary
Node* unary(){
  if(consume(std::string("+").c_str())){
    //printf("unary() +\n");
    return unary();
  }
  if(consume(std::string("-").c_str())){
    return new_binary(ND_SUB, new_num(0), unary());
  }
  return primary();

} //urary()

// primary = "(" expr ")" | num | ident
Node* primary() {
  if(consume(std::string("(").c_str())) {
    Node *node = expr();
    expect(std::string(")").c_str());
    return node;
  } //if consume("(")

  Token* tok = consume_ident();
  if(tok){
    Node* node = (Node*)calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar* lvar = find_lvar(tok);
    if(lvar){
      node->lvar = lvar;
      node->offset = lvar->offset;
    } else {
      lvar = (LVar*)calloc(1, sizeof(LVar));
      //lvar->next = locals;
      locals.push_back(lvar);
      lvar->name = tok->str;
      lvar->len = tok->len;
      
      if(!locals.empty()){
	lvar->offset = locals.front()->offset + 8;
      } else {
	lvar->offset = 8;
      } //if
      node->offset = lvar->offset;
      node->lvar = lvar;
      //locals = lvar;
    } //if
    return node;
} //if tok

  return new_num(expect_number());
} //primary()

