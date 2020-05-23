#include "jcc.h"

static const std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
static const std::string regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};

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
  } //switch 
} //gen()

void gen_last(const int d){
  printf("  mov rax, %s\n", regs[d].c_str());
}
