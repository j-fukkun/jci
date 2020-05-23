#include "jcc.h"

char* user_input;
Token* token;
std::list<IR*> IR_list;

//
//IR dump
//
void IR_dump(){

  for(auto iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    IR* ir = *iter;
    //const int d = ir->d->vn;
    //const int a = ir->a->vn;
    //const int b = ir->b->vn;
    
    switch(ir->opcode){
    case IR_MOV:
      //printf("v%d = v%d\n", d, b);
      printf("MOV\n");
      break;
    case IR_ADD:
      //printf("v%d = v%d + v%d", d, a, b);
      printf("ADD\n");
      break;
    case IR_SUB:
      //printf("v%d = v%d - v%d", d, a, b);
      printf("SUB\n");
      break;
    case IR_MUL:
      //printf("v%d = v%d * v%d", d, a, b);
      printf("MUL\n");
      break;
    case IR_DIV:
      //printf("v%d = v%d / v%d", d, a, b);
      printf("DIV\n");
      break;
    default:
      break;
    } //switch
  } //for
  
} //IR_dump()

void ND_dump(Node* node){

  switch(node->kind){
  case ND_ADD:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_ADD\n");
    break;
  case ND_SUB:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_SUB\n");
    break;
  case ND_MUL:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_MUL\n");
    break;
  case ND_DIV:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_DIV\n");
    break;
  case ND_NUM:
    printf("ND_NUM %d\n", node->val);
    break;
  case ND_EQ:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_EQ\n");
    break;
  case ND_NE:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_NE\n");
    break;
  case ND_LT:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_LT\n");
    break;
  case ND_LE:
    ND_dump(node->lhs);
    ND_dump(node->rhs);
    printf("ND_LE\n");
    break;
  }
  
} //ND_dump


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

  //ND_dump(node);

  /*アセンブリの前半部分を出力*/
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  Reg* reg = gen_expr_IR(node);

  //IR_dump();
  
  convertThreeToTwo();
  
  const bool allocated = allocateRegister();
  assert(allocated);
  //IR_dump();
  
  for(auto iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    gen(*iter);
  }

  auto temp = IR_list.end();
  temp--;
  int d = (*temp)->d->rn;
  gen_last(d);
  
  //printf("  pop rax\n");
  printf("  ret\n");
  return 0;

} /*main*/
