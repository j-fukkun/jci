#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>
//#include <iostream>

//
//Tokenizer
//

/* トークンの種類*/
enum TokenKind{
  TK_RESERVED, /*記号*/
  TK_NUM, /*整数*/
  TK_EOF, /*入力の終わりを表すトークン*/
};

//typedef struct Token Token;
/*トークン型*/
struct Token{
  TokenKind kind; /*トークンの型*/
  Token *next; /*次の入力トークン*/
  int val; /*kindがTK_NUMの場合、その数値*/
  char *str; /*トークン文字列*/
};

//input program
char* user_input;

/*current token*/
Token *token;

/*エラーを報告するための関数*/
/*prnitfと同じ引数を取る*/
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/*次のトークンが期待している記号のときには、トークンを一つ読み進めて*/
/*真を返す。それ以外の場合は、偽を返す*/
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

/*次のトークンが期待している記号のときには、トークンを一つ読み進める。*/
/*それ以外の場合には、エラーを報告する*/
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op){
    /*error("'%c'ではありません", op);*/
    char msg[] = "expected '%c'";
    error_at(token->str, msg, op);
  }
  token = token->next;
}

/*次のトークンが数値の場合、トークンを一つ読み進めて、その数値を返す。*/
/*それ以外の場合には、エラーを報告する*/
int expect_number() {
  if (token->kind != TK_NUM){
    /*error("数ではありません");*/
    char msg[] = "expected a number";
    error_at(token->str, msg);
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

/*新しいトークンを作成してcurにつなげる*/
Token* new_token(TokenKind kind, Token *cur, char *str) {
  Token* tok = (Token*)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

/*入力文字列pをトークナイズしてそれを返す*/
Token* tokenize() {
  char* p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    /*空白文字をスキップ*/
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strchr("+-*/()", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    /*error("invalid token");*/
    char msg[] = "invalid token";
    error_at(p, msg);
  } //while()

  new_token(TK_EOF, cur, p);
  return head.next;
}


//
//Parser
//

enum NodeKind{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // Integer
};

// AST node type
//typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node* lhs;     // Left-hand side
  Node* rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
};

Node* new_node(NodeKind kind) {
  Node* node = (Node*)calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node* new_binary(NodeKind kind, Node* lhs, Node* rhs) {
  Node* node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

Node *expr();
Node *mul();
Node *primary();

// expr = mul ("+" mul | "-" mul)*
Node *expr() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_binary(ND_ADD, node, mul());
    else if (consume('-'))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = primary ("*" primary | "/" primary)*
Node *mul() {
  Node *node = primary();

  for (;;) {
    if (consume('*'))
      node = new_binary(ND_MUL, node, primary());
    else if (consume('/'))
      node = new_binary(ND_DIV, node, primary());
    else
      return node;
  }
}

// primary = "(" expr ")" | num
Node *primary() {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }

  return new_num(expect_number());
}

//
//IR generator
//

enum IRKind{
  IR_ADD, // +
  IR_SUB, // -
  IR_MUL, // *
  IR_DIV, // /
  IR_IMM, // immediate value
  IR_MOV, // mov
};

struct Reg{

  int vn; //virtual register number
  int rn; //real register number

  //for register allocater
  bool def;
};

struct IR{
  IRKind opcode; //operation code  d = a op b
  Reg* d; //destination operand
  Reg* a; //source operand left
  Reg* b; //source operand right 

  int imm; //immediate value
};

std::list<IR*> IR_list;

IR* new_ir(IRKind opcode){
  IR* ir = (IR*)calloc(1, sizeof(IR));
  ir->opcode = opcode;
  IR_list.push_back(ir);
  return ir;
}

static int nreg = 1;
Reg* new_reg(){
  Reg* reg = (Reg*)calloc(1, sizeof(Reg));
  reg->vn = nreg++;
  reg->rn = -1;
  return reg;
}

Reg* new_imm(int imm){
  Reg* reg = new_reg();
  IR* ir = new_ir(IR_IMM);
  ir->d = reg;
  ir->imm = imm;
  return reg;
}

IR* emit_IR(IRKind op, Reg* d, Reg* a, Reg* b){
  IR* ir = new_ir(op);
  ir->d = d;
  ir->a = a;
  ir->b = b;
  return ir;
}

Reg* gen_expr_IR(Node* node);

Reg* gen_binop_IR(IRKind op, Node* node){
  Reg* d = new_reg();
  Reg* a = gen_expr_IR(node->lhs);
  Reg* b = gen_expr_IR(node->rhs);
  emit_IR(op, d, a, b);
  return d;
}

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
  } //switch
}

//
//register allocater
//
void convertThreeToTwo(){
  //convert d = a op b; --> mov d, a; op d, b;
  std::list<IR*> vec;

  for(std::list<IR*>::iterator iter = IR_list.begin(), end = IR_list.end(); iter != end; iter++){
    IR* ir = *iter;
    if(!ir->d || !ir->a){
      vec.push_back(*iter);
      continue;
    } //if

    IR* ir2 = (IR*)calloc(1, sizeof(IR));
    ir2->opcode = IR_MOV;
    ir2->d = ir->d;
    ir2->b = ir->a;
    vec.push_back(ir2);

    ir->a = ir->d;
    vec.push_back(ir);
  } //iter
  IR_list = vec;
} //convertThreeToTwo

const std::vector<Reg*> collect_reg(){
  std::vector<Reg*> vec;

  for(std::list<IR*>::iterator iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    IR* ir = *iter;
    if(ir->d && !ir->d->def){
      ir->d->def = true;
      vec.push_back(ir->d);
    } //if
  } //for
  return vec;
} //collect_reg()

std::unordered_map<int, Reg*> used;
//std::unordered_map<int, int> mapVtoR;
const int num_reg = 7;

const bool allocateRegister(){

  const std::vector<Reg*> vec_reg = collect_reg();
  
  for(int i = 0; i < num_reg; i++){
    used[i] = nullptr;
  } //for
  
  for(auto iter = vec_reg.begin(), end = vec_reg.end(); iter != end; ++iter){
    Reg* reg = *iter;
    bool found = false;
    for(int i = 0; i < num_reg; i++){
      if(used[i] /*|| mapVtoR[reg->vn]*/){
	continue;
      }
      reg->rn = i;
      used[i] = reg;
      //mapVtoR[reg->vn] = i;
      found = true;
      break;
    } //for i

    /*
    if(found){
      continue;
    } else {
      return false;
    } //if
    */
  } //for iter

  return true;
  
} //allocateRegister



//
// code generator
//
std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};

void gen(IR* ir) {
  int d = ir->d ? ir->d->rn : 0;
  int a = ir->a ? ir->a->rn : 0;
  int b = ir->b ? ir->b->rn : 0;

  switch(ir->opcode){
  case IR_IMM:
    printf("  mov %s, %d\n", regs[d].c_str(), ir->imm);
    break;
  case IR_MOV:
    printf("  mov %s, %s\n", regs[d].c_str(), regs[b].c_str());
    break;
  case IR_ADD:
    printf("  add %s, %s\n", regs[d].c_str(), regs[b].c_str());
    break;
  case IR_SUB:
    printf("  sub %s, %s\n", regs[d].c_str(), regs[b].c_str());
    break;
  case IR_MUL:
    printf("  mov rax, %s\n", regs[b].c_str());
    printf("  imul %s\n", regs[d].c_str());
    printf("  mov %s, rax\n", regs[d].c_str());
    break;
  case IR_DIV:
    printf("  mov rax, %s\n", regs[d].c_str());
    printf("  cqo\n");
    printf("  idiv %s\n", regs[b].c_str());
    printf("  mov %s, rax\n", regs[d].c_str());
    break;
  } //switch 
}

//
//IR dump
//
void IR_dump(){

  for(auto iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    IR* ir = *iter;
    const int d = ir->d->vn;
    const int a = ir->a->vn;
    const int b = ir->b->vn;
    
    switch(ir->opcode){
    case IR_MOV:
      printf("v%d = v%d\n", d, b);
      break;
    case IR_ADD:
      printf("v%d = v%d + v%d", d, a, b);
      break;
    case IR_SUB:
      printf("v%d = v%d - v%d", d, a, b);
      break;
    case IR_MUL:
      printf("v%d = v%d * v%d", d, a, b);
      break;
    case IR_DIV:
      printf("v%d = v%d / v%d", d, a, b);
      break;
    default:
      break;
    } //switch
  } //for
  
} //IR_dump()


int main(int argc, char **argv){
  if(argc != 2){
    char msg[] = "%s: invalid number of argument";
    error(msg, argv[0]);
    //return 1;
  }

  /*tokenize*/
  user_input = argv[1];
  token = tokenize();
  Node* node = expr();

  /*アセンブリの前半部分を出力*/
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  Reg* reg = gen_expr_IR(node);

  //IR_dump();
  
  convertThreeToTwo();

  //IR_dump();
  
  const bool allocated = allocateRegister();
  assert(allocated);
  
  for(auto iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    gen(*iter);
  }

  auto temp = IR_list.end();
  temp--;
  int d = (*temp)->d->rn;
  printf("  mov rax, %s\n", regs[d].c_str());
  
  //printf("  pop rax\n");
  printf("  ret\n");
  return 0;

} /*main*/
