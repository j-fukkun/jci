#ifndef JCC_H
#define JCC_H

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
  int len; //トークンの長さ
};

//input program
extern char* user_input;

/*current token*/
extern Token* token;

void error(char* fmt, ...);
void error_at(char* loc, char* fmt, ...);
bool consume(const char* op);
void expect(const char* op);
const int expect_number();
const bool at_eof();
const bool startswith(char* p, const char* q);
Token* tokenize();


//
//Parser
//

enum NodeKind{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // Integer
  ND_EQ, //==
  ND_NE, //!=
  ND_LT, //<
  ND_LE, //<=
};

// AST node type
//typedef struct Node Node;
struct Node {
  NodeKind kind; // Node kind
  Node* lhs;     // Left-hand side
  Node* rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
};


Node* new_node(const NodeKind kind);
Node* new_binary(const NodeKind kind, Node* lhs, Node* rhs);
Node* new_num(const int val);
Node* expr();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();


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
  IR_EQ, //==
  IR_NE, //!=
  IR_LT, //<
  IR_LE, //<=
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

extern std::list<IR*> IR_list;

IR* new_ir(const IRKind opcode);
Reg* new_reg();
Reg* new_imm(const int imm);
IR* emit_IR(const IRKind op, Reg* d, Reg* a, Reg* b);
Reg* gen_binop_IR(const IRKind op, Node* node);
Reg* gen_expr_IR(Node* node);


//
//register allocator
//

void convertThreeToTwo();
const std::vector<Reg*> collect_reg();
const bool allocateRegister();


//
// code generator
//
void print_cmp(const std::string inst, const IR* ir);
void gen(const IR* ir);
void gen_last(const int d);

#endif
