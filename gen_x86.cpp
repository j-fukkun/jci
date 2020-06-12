#include "jcc.h"

static const std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
static const std::string regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
static const std::string argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static unsigned int labelseq = 1;

void print_cmp(const std::string inst, const IR* ir){
  //EQ,NE,LT,LEの命令を出力
  const int d = ir->d->rn;
  const int a = ir->a->rn;
  const int b = ir->b->rn;

  printf("  cmp %s, %s\n", regs[a].c_str(), regs[b].c_str()); //compare フラグレジスタに結果が格納される
  printf("  %s %s\n", inst.c_str(), regs8[d].c_str()); //フラグレジスタに格納された結果を8ビットレジスタに格納
  printf("  movzb %s, %s\n", regs[d].c_str(), regs8[d].c_str()); //64bitレジスタの上位56bitをゼロクリア
  
} //print_cmp()


void gen(const IR* ir){
  const int d = ir->d ? ir->d->rn : 0;
  const int a = ir->a ? ir->a->rn : 0;
  const int b = ir->b ? ir->b->rn : 0;

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
  case IR_EQ:
    print_cmp(std::string("sete"), ir);
    break;
  case IR_NE:
    print_cmp(std::string("setne"), ir);
    break;
  case IR_LT:
    print_cmp(std::string("setl"), ir);
    break;
  case IR_LE:
    print_cmp(std::string("setle"), ir);
    break;
  case IR_LVAR:
    printf("  lea %s, [rbp-%d]\n", regs[d].c_str(), ir->lvar->offset);
    break;
  case IR_RETURN:
    printf("  mov rax, %s\n", regs[a].c_str());
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    //printf("  jmp %s\n", ret);
    break;
  case IR_LOAD:
    printf("  mov %s, [%s]\n", regs[d].c_str(), regs[b].c_str());
    break;
  case IR_LOAD_SPILL:
    printf("  mov %s, [rbp-%d]", regs[d].c_str(), ir->lvar->offset);
    break;
  case IR_STORE:
    printf("  mov [%s], %s\n", regs[a].c_str(), regs[b].c_str());
    break;
  case IR_STORE_SPILL:
    printf("  mov [rbp-%d], %s", ir->lvar->offset, regs[a].c_str());
    break;
  case IR_BR:
    printf("  cmp %s, 0\n", regs[b].c_str());
    printf("  jne .L%d\n", ir->bb1->label);
    printf("  jmp .L%d\n", ir->bb2->label);
    break;
  case IR_JMP:
    /*
    if (ir->bbarg){
      printf("  mov %s, %s", regs[ir->bb1->param->rn], regs[ir->bbarg->rn]);
    } //if
    */
    printf("  jmp .L%d\n", ir->bb1->label);
    break;
  case IR_FUNCALL:
    /*
    int seq = labelseq++;
    printf("  mov rax, rsp\n");
    printf("  and rax, 15\n"); //15(1111)とand演算
    //16は(0001 0000)なので、15とand演算すると、0になる
    //演算結果が0のとき、ZFには1がセットされる
    printf("  jnz .L.call.%d\n", seq); //ZF(Zero Flag) = 0のときにjump
    printf("  push r10\n");
    printf("  push r11\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", ir->funcname);
    printf("  jmp .L.end.%d\n", seq);
    printf(".L.call.%d:\n", seq); //RSPが16の倍数ではないとき
    printf("  sub rsp, 8\n");
    printf("  push r10\n");
    printf("  push r11\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", ir->funcname);
    printf("  add rsp, 8\n");
    printf(".L.end.%d:\n", seq);
    printf("  pop r11\n");
    printf("  pop r10\n");
    printf("  mov %s, rax\n", regs[d].c_str());
    */
    
    for (int i = 0; i < ir->num_args; i++){
      printf("  mov %s, %s\n", argregs[i].c_str(), regs[ir->args[i]->rn].c_str());
    } //for
    
    printf("  push r10\n");
    printf("  push r11\n");
    printf("  mov rax, 0\n");
    printf("  call %s\n", ir->funcname);
    printf("  pop r11\n");
    printf("  pop r10\n");
    printf("  mov %s, rax\n", regs[d].c_str());
    break;
  } //switch 
} //gen()

void gen_last(const int d){
  printf("  mov rax, %s\n", regs[d].c_str());
}
