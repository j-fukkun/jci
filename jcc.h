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
#include <climits>

//
//Tokenizer
//

/* トークンの種類*/
enum TokenKind{
  TK_RESERVED, /*記号*/
  TK_NUM, /*整数*/
  TK_IDENT, //識別子
  TK_STR, //文字列
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

  char* strings; //string literal including '\0'
  int str_len; //string literal length
};

//input program
extern char* user_input;

/*current token*/
extern Token* token;

//input filename
extern char* filename;

void error(const char* fmt, ...);
void error_at(char* loc, const char* fmt, ...);
void error_tok(Token* tok, const char* fmt, ...);
void warn_tok(Token* tok, const char* fmt, ...);
bool consume(const char* op);
Token* consume_ident();
Token* consume_str();
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
  ND_VAR, //local or global variant
  ND_RETURN, //return
  ND_IF, //if
  ND_WHILE, //while
  ND_FOR, //for
  ND_BLOCK, //{}
  ND_FUNCALL, //function call
  ND_DEREF, //rereference *
  ND_ADDR, //address &
  ND_PTR_ADD, //pointer add
  ND_PTR_SUB, //pointer sub
  ND_PTR_DIFF, //pointer difference
  ND_PRE_INC, //++a
  ND_PRE_DEC, //--a
  //ND_POST_INC, //a++
  //ND_POST_DEC, //a--
  ND_STMT_EXPR, //stmt ...
  ND_EXPR_STMT, //expr ...
  ND_LOGOR, //||
  ND_LOGAND, //&&
  ND_SHL, //<< left shift
  ND_SHR, //>> right shift
  ND_NULL,
};

struct Type;
struct Initializer;

//type for variable
struct Var{
  Var* next; //次の変数 or NULL
  const char* name; //変数の名前
  int len; //変数名の長さ
  int offset; //RBPからのオフセット for local variable
  Type* type;
  bool is_local; //if this is true then local var else global var
  bool is_literal; //if this is true then string literal

  //for global variable
  char* literal; //string literal

  Initializer* initializer;
};

// AST node type
//typedef struct Node Node;
struct Node{
  NodeKind kind; // Node kind
  Node* lhs;     // Left-hand side
  Node* rhs;     // Right-hand side
  int val;       // Used if kind == ND_NUM

  Var* var;

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

  Type* type;

  Node* expr;
  std::vector<Node*> stmt;
};

struct Initializer{
  Initializer* next;

  //constant expression
  int size;
  int val;
};

struct Designator{
  Designator* next;
  int index; //for array
};

class BasicBlock;
class Function{
 public:
  Function* next;
  char* name;
  //Var* params;
  std::list<Var*> params;
  std::list<BasicBlock*> bbs;

  Node* node; //function body
  Var* locals; //local variables in function
  int stack_size;
};

struct Program{
  Function* fns;
  Var* globals;
};


extern Type* int_type;
extern Type* char_type;
extern Type* void_type;

enum TypeKind{
  TY_INT, //4 byte
  TY_CHAR, //1 byte
  TY_SHORT,  //2 byte
  TY_LONG, //8 byte
  TY_VOID,
  TY_PTR,
  TY_ARRAY,
};

class Type{
 public:
  TypeKind kind;
  int size;
  int align;
  Type* base;    //pointer
  int array_size; //size of array
  bool is_incomplete; //index is omitted?

  Type(){}
  //Type(TypeKind k){kind = k;}
  Type(TypeKind k, int sz, int ali)
    {kind = k; size = sz; align = ali; base = nullptr;
      is_incomplete = false;
    }
  ~Type(){}
};

bool is_integer(Type* t);
Type* pointer_to(Type* base);
Type* array_of(Type* base, int size);
const int align_to(const int n, const int align);
void add_type(Node* node);


extern Var* locals;
extern Var* globals;
extern int nlabel;

Var* find_lvar(Token* tok);
Node* new_node(const NodeKind kind);
Node* new_binary(const NodeKind kind, Node* lhs, Node* rhs);
Node* new_num(const int val);

Program* program();
Function* function();
Type* type_suffix(Type*);
const int const_expr();
const int eval(Node*);
Node* stmt();
Node* stmt2();
Node* expr();
Node* assign();
Node* logor();
Node* logand();
Node* equality();
Node* relational();
Node* shift();
Node* new_add(Node* lhs, Node* rhs);
Node* new_sub(Node* lhs, Node* rhs);
Node* add();
Node* mul();
Node* new_unary(NodeKind kind, Node* lhs);
Node* unary();
Node* postfix();
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
  IR_PTR_ADD, //pointer add
  IR_PTR_SUB, //pointer sub
  IR_PTR_DIFF, //pointer difference
  IR_LABEL_ADDR, //global variable
  IR_SHL, //<< left shift
  IR_SHR, //>> right shift
  
};

class Reg{
 public:
  int vn; //virtual register number
  int rn; //real register number

  //for register allocater
  int def;
  int last_use;
  bool spill;
  Var* lvar;
};

class IR;
class BasicBlock{
 public:
  int label;
  std::list<IR*> instructions;

  // For liveness analysis
  std::list<BasicBlock*> succ;
  std::list<BasicBlock*> pred;

  Reg* param;
};

class IR{
 public:
  IRKind opcode; //operation code  d = a op b
  Reg* d; //destination operand
  Reg* a; //source operand left
  Reg* b; //source operand right 

  int imm; //immediate value
  Var* lvar;
  const char* name; //global var name

  BasicBlock* bb1;
  BasicBlock* bb2;
  Reg* bbarg;

  char* funcname; //function name
  Reg* args[6]; //arguments
  int num_args; //the number of arguments

  int type_size;
  int type_base_size;
};


IR* new_ir(const IRKind opcode);
Reg* new_reg();
Reg* new_imm(const int imm);
IR* emit_IR(const IRKind op, Reg* d, Reg* a, Reg* b);
Reg* gen_binop_IR(const IRKind op, Node* node);
Reg* gen_expr_IR(Node* node);
void gen_IR(Program* prog);
void dump_IR(Program* prog);

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
