#include "jcc.h"

char* user_input;
Token* token;
Node* ir_code[100];
std::list<IR*> IR_list;
std::list<BasicBlock*> BB_list;

//
//IR dump
//
void IR_dump(){

  for(auto iter = BB_list.begin(), end = BB_list.end(); iter != end; ++iter){
    
    for(auto it_inst = (*iter)->instructions.begin(), end_inst = (*iter)->instructions.end(); it_inst != end_inst; ++it_inst){
      IR* ir = *it_inst;
      //const int d = ir->d->vn;
      //const int a = ir->a->vn;
      //const int b = ir->b->vn;
      
      switch(ir->opcode){
      case IR_IMM:
	printf("v%d = %d\n", ir->d->vn, ir->imm);
	break;
      case IR_MOV:
	printf("v%d = v%d\n", ir->d->vn, ir->b->vn);
	//printf("MOV\n");
	break;
      case IR_ADD:
	printf("v%d = v%d + v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	//printf("ADD\n");
	break;
      case IR_SUB:
	printf("v%d = v%d - v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	//printf("SUB\n");
	break;
      case IR_MUL:
	printf("v%d = v%d * v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	//printf("MUL\n");
	break;
      case IR_DIV:
	printf("v%d = v%d / v%d\n", ir->d->vn, ir->a->vn, ir->b->vn);
	//printf("DIV\n");
	break;
      case IR_LVAR:
	printf("Load v%d [rbp-%d]\n", ir->d->vn, ir->lvar->offset);
	break;
      case IR_STORE:
	printf("Store [v%d] v%d\n", ir->a->vn, ir->b->vn);
	break;
      case IR_LOAD:
	printf("Load v%d [v%d]\n", ir->a->vn, ir->b->vn);
	break;
      case IR_RETURN:
	printf("RETURN v%d\n", ir->a->vn);
      default:
	break;
      } //switch
    } //for it_inst
  } //for iter
  
} //IR_dump()


int main(int argc, char **argv){
  if(argc != 2){
    char msg[] = "%s: invalid number of argument";
    error(msg, argv[0]);
  }

  //tokenize
  user_input = argv[1];
  token = tokenize();
  program();

  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  //プロローグ
  //変数26個文の領域を確保する
  printf("  push rbp\n");;
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  gen_IR();

  //IR_dump();
  
  allocateRegister();
  //IR_dump();
  
  for(auto iter = BB_list.begin(), end = BB_list.end(); iter != end; ++iter){
    printf(".L%d:\n", (*iter)->label);
    for(auto it_inst = (*iter)->instructions.begin(), end_inst = (*iter)->instructions.end(); it_inst != end_inst; ++it_inst){
      gen(*it_inst);
    } //for
  } //for
  
  return 0;

} //main
