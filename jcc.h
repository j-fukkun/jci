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

//
//Tokenizer
//

/* トークンの種類*/
enum TokenKind{
  TK_RESERVED, /*記号*/
  TK_NUM, /*整数*/
  TK_IDENT, //識別子
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
void error_at(char* loc, const char* fmt, ...);
bool consume(const char* op);
Token* consume_ident();
void expect(const char* op);
const int expect_number();
char* expect_ident();
Token* peek(const char* s);
const bool at_eof();
Token* new_token(TokenKind kind, Token* cur, char* str, int len);
const bool is_alphabet(char c);
const bool is_alphabet_or_number(char c);
const bool startswith(char* p, const char* q);
const char* startswith_reserved(char* p);
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
  ND_ASSIGN, //assignment
  ND_LVAR, //local variant
  ND_RETURN, //return
  ND_IF, //if
  ND_WHILE, //while
  ND_FOR, //for
  ND_BLOCK, //{}
  ND_FUNCALL, //function call
  ND_DEREF, //rereference *
  ND_ADDR, //address &
  ND_NULL,
};

//type for local variable
struct LVar{
  LVar* next; //次の変数 or NULL
  char* name; //変数の名前
  int len; //変数名の長さ
  int offset; //RBPからのオフセット
};

// AST node type
//typedef struct Node Node;
struct Node{
  NodeKind kind; // Node kind
  Node* lhs;     // Left-hand side
  Node* rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM
  int offset;    // Used if kind == ND_LVAR

  LVar* lvar;

  //"if","while","for"
  Node* cond;
  Node* then;
  Node* els;
  Node* init; //for
  Node* inc; //for

  //block
  Node* body;
  Node* next;

  //function call
  char* funcname; //function name
  Node* args; //function arguments
};

class BasicBlock;
class Function{
 public:
  Function* next;
  char* name;
  LVar* params;
  std::list<BasicBlock*> bbs;

  Node* node; //function body
  LVar* locals; //local variables in function
  int stack_size;
};

struct Program{
  Function* fns;
};


extern LVar* locals;
extern int nlabel;

LVar* find_lvar(Token* tok);
Node* new_node(const NodeKind kind);
Node* new_binary(const NodeKind kind, Node* lhs, Node* rhs);
Node* new_num(const int val);

Program* program();
Function* function();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* primary();

//extern Node* ir_code[100];

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
  IR_LVAR, //local var
  IR_STORE, //store
  IR_STORE_SPILL,
  IR_STORE_ARG, //function params
  IR_LOAD,  //load
  IR_LOAD_SPILL,
  IR_RETURN, //return
  IR_BR, //branch
  IR_JMP, //jump
  IR_FUNCALL, //function call
};

class Reg{
 public:
  int vn; //virtual register number
  int rn; //real register number

  //for register allocater
  int def;
  int last_use;
  bool spill;
  LVar* lvar;
};

class IR;
class BasicBlock{
 public:
  int label;
  std::list<IR*> instructions;

  // For liveness analysis
  std::list<BasicBlock*> succ;
  std::list<BasicBlock*> pred;
};

class IR{
 public:
  IRKind opcode; //operation code  d = a op b
  Reg* d; //destination operand
  Reg* a; //source operand left
  Reg* b; //source operand right 

  int imm; //immediate value
  LVar* lvar;

  BasicBlock* bb1;
  BasicBlock* bb2;
  //Reg* bbarg;

  char* funcname; //function name
  Reg* args[6]; //arguments
  int num_args; //the number of arguments
};

//extern std::list<IR*> IR_list;
//extern std::list<BasicBlock*> BB_list;

IR* new_ir(const IRKind opcode);
Reg* new_reg();
Reg* new_imm(const int imm);
IR* emit_IR(const IRKind op, Reg* d, Reg* a, Reg* b);
Reg* gen_binop_IR(const IRKind op, Node* node);
Reg* gen_expr_IR(Node* node);
void gen_IR(Program* prog);


//
//register allocator
//

//void convertThreeToTwo();
//const std::vector<Reg*> collect_reg();
//const bool allocateRegister();
void allocateRegister(Program* prog);


//
// code generator
//
void gen_x86(Program* prog);

#endif
