#include "jcc.h"


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

// expr = equality
Node* expr(){
  return equality(); 
} //expr()

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

// primary = "(" expr ")" | num
Node* primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(std::string(")").c_str());
    return node;
  }

  return new_num(expect_number());
} //primary()

