#include "jcc.h"

static const std::string regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"}; //64bit 8byte
static const std::string regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"}; //8bit 1byte
static const std::string regs16[] = {"r10w", "r11w", "bx", "r12w", "r13w", "r14w", "r15w"}; //16bit 2byte
static const std::string regs32[] = {"r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"}; //32bit 4byte

static const std::string argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}; //64bit 8byte
static const std::string argregs8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"}; //8bit 1byte
static const std::string argregs16[] = {"di", "si", "dx", "cx", "r8w", "r9w"}; //16bit 2byte
static const std::string argregs32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"}; //32bit 4byte

static unsigned int labelseq = 1;
static char* funcname;

int roundup(const int x, const int align) {
  return (x + align - 1) & ~(align - 1);
}

void print_cmp(const std::string inst, const IR* ir){
  //EQ,NE,LT,LEの命令を出力
  const int d = ir->d->rn;
  const int a = ir->a->rn;
  const int b = ir->b->rn;

  if(ir->b->isImm){
    printf("  cmp %s, %d\n", regs[a].c_str(), ir->b->imm);
  } else {
    printf("  cmp %s, %s\n", regs[a].c_str(), regs[b].c_str()); //compare フラグレジスタに結果が格納される
  }
  printf("  %s %s\n", inst.c_str(), regs8[d].c_str()); //フラグレジスタに格納された結果を8ビットレジスタに格納
  printf("  movzb %s, %s\n", regs[d].c_str(), regs8[d].c_str()); //64bitレジスタの上位56bitをゼロクリア
  
} //print_cmp()

const std::string reg(const int r, const int size){
  fflush(stdout);
  assert(r >= 0);
  if(size == 1){
    return regs8[r];
  }

  if(size == 2){
    return regs16[r];
  }
  
  if(size == 4){
    return regs32[r];
  }
  
  assert(size == 8);
  return regs[r];
  
} //reg()

const std::string argreg(const int r, const int size){
  
  if(size == 1){
    return argregs8[r];
  }

  if(size == 2){
    return argregs16[r];
  }
  
  if(size == 4){
    return argregs32[r];
  }
  assert(size == 8);
  return argregs[r];
  
} //argreg()

const std::string sizeQualifier(const int size){
  if(size == 1){
    return std::string("BYTE PTR");
  }

  if(size == 2){
    return std::string("WORD PTR");
  }

  if(size == 4){
    return std::string("DWORD PTR");
  }
  
  assert(size == 8);
  return std::string("QWORD PTR");
}

void load(const IR* ir){
  const int d = ir->d ? ir->d->rn : 0;
  const int b = ir->b ? ir->b->rn : 0;
  const int size = ir->type_size;

  if(size == 1){
    printf("  movsx %s, byte ptr [%s]\n", regs[d].c_str(), regs[b].c_str());
  } else if(size == 2){
    printf("  movsx %s, word ptr [%s]\n", regs[d].c_str(), regs[b].c_str());
  } else if(size == 4){
    printf("  movsxd %s, dword ptr [%s]\n", regs[d].c_str(), regs[b].c_str());
  } else {
    assert(size == 8);
    printf("  mov %s, [%s]\n", regs[d].c_str(), regs[b].c_str());
  } //if
  
}

void truncate(const IR* ir){
  Type* type = ir->type;
  const int a = ir->a ? ir->a->rn : 0;
  
  if(type->kind == TY_BOOL){
    printf("  cmp %s, 0\n", regs[a].c_str());
    printf("  setne %s\n", regs8[a].c_str());
  } //if

  if(type->size == 1){
    printf("  movsx %s, %s\n", regs[a].c_str(), regs8[a].c_str());
  } else if(type->size == 2){
    printf("  movsx %s, %s\n", regs[a].c_str(), regs16[a].c_str());
  } else if(type->size == 4){
    printf("  movsxd %s, %s\n", regs[a].c_str(), regs32[a].c_str());
  } else if(type->size == 8){
    printf("  mov %s, %s\n", regs[a].c_str(), regs[a].c_str());
  } //if
  
} //truncate()

void gen(const IR* ir){
  const int d = ir->d ? ir->d->rn : 0;
  const int a = ir->a ? ir->a->rn : 0;
  const int b = ir->b ? ir->b->rn : 0;

  switch(ir->opcode){
  case IR_IMM:
    printf("  mov %s, %d\n", regs[d].c_str(), ir->imm);
    break;
  case IR_MOV:
    if(ir->b->isImm){
      printf("  mov %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  mov %s, %s\n", regs[d].c_str(), regs[b].c_str());      
    }
    break;
  case IR_ADD:
    if(ir->b->isImm){
      printf("  add %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  add %s, %s\n", regs[d].c_str(), regs[b].c_str());
    } 
    break;
  case IR_SUB:
    if(ir->b->isImm){
      printf("  sub %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  sub %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_MUL:
    if(ir->b->isImm){
      printf("  mov rax, %d\n", ir->b->imm);
    } else {
      printf("  mov rax, %s\n", regs[b].c_str());
    }
    printf("  imul %s\n", regs[d].c_str());
    printf("  mov %s, rax\n", regs[d].c_str());
    break;
  case IR_DIV:
    printf("  mov rax, %s\n", regs[d].c_str());
    printf("  cqo\n");
    /*
    if(ir->b->isImm){
      printf("  idiv %d\n", ir->b->imm);
    } else {
    */
      printf("  idiv %s\n", regs[b].c_str());
      //}
    printf("  mov %s, rax\n", regs[d].c_str());
    break;
  case IR_MOD:
    printf("  mov rax, %s\n", regs[d].c_str());
    printf("  cqo\n");
    /*
    if(ir->b->isImm){
      printf("  idiv %d\n", ir->b->imm);
    } else {
    */
      printf("  idiv %s\n", regs[b].c_str());
      //}
    printf("  mov %s, rdx\n", regs[d].c_str());
    break;
  case IR_CAST:
    truncate(ir);
    break;
  case IR_PTR_ADD:
    if(ir->b->isImm){
      printf("  add %s, %d\n", regs[d].c_str(), ir->b->imm * ir->type_base_size);
    } else {
      printf("  imul %s, %d\n", regs[b].c_str(), ir->type_base_size);
      printf("  add %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_PTR_SUB:
    if(ir->b->isImm){
      printf("  sub %s, %d\n", regs[d].c_str(), ir->b->imm * ir->type_base_size);
    } else {
      printf("  imul %s, %d\n", regs[b].c_str(), ir->type_base_size);
      printf("  sub %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_PTR_DIFF:
    printf("  sub %s, %s\n", regs[d].c_str(), regs[b].c_str());
    printf("  cqo\n");
    printf("  mov %s, %d\n", regs[b].c_str(), ir->type_base_size);
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
  case IR_SHL:
    if(ir->b->isImm){
      printf("  mov cl, %d\n", ir->b->imm);
    } else {
      printf("  mov cl, %s\n", regs8[b].c_str());
    }
    printf("  shl %s, cl\n", regs[d].c_str());
    break;
  case IR_SHR:
    if(ir->b->isImm){
      printf("  mov cl, %d\n", ir->b->imm);
    } else {
      printf("  mov cl, %s\n", regs8[b].c_str());
    }
    printf("  shr %s, cl\n", regs[d].c_str());
    break;
  case IR_BITOR:
    if(ir->b->isImm){
      printf("  or %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  or %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_BITAND:
    if(ir->b->isImm){
      printf("  and %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  and %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_BITXOR:
    if(ir->b->isImm){
      printf("  xor %s, %d\n", regs[d].c_str(), ir->b->imm);
    } else {
      printf("  xor %s, %s\n", regs[d].c_str(), regs[b].c_str());
    }
    break;
  case IR_LVAR:
    printf("  lea %s, [rbp-%d]\n", regs[d].c_str(), ir->lvar->offset);
    break;
  case IR_RETURN:
    if(ir->a != nullptr){
      if(ir->a->isImm){
	printf("  mov rax, %d\n", ir->a->imm);
      } else {
	printf("  mov rax, %s\n", regs[a].c_str());
      }
    }
    printf("  jmp .L.return.%s\n", funcname);
    break;
  case IR_LOAD:
    //printf("  mov %s, [%s]\n", reg(d, ir->type_size).c_str(), regs[b].c_str());
    load(ir);
    if(ir->type_size == 1){
      printf("  movzb %s, %s\n", regs[d].c_str(), regs8[d].c_str());
    } //if
    break;
  case IR_LOAD_SPILL:
    printf("  mov %s, [rbp-%d]\n", regs[d].c_str(), ir->lvar->offset);
    break;
  case IR_STORE:
    if(ir->type->kind == TY_BOOL){
      if(ir->b->isImm){
	if(ir->b->imm == 0){
	  printf("  mov BYTE PTR [%s], 0\n", regs[a].c_str());
	} else {
	  printf("  mov BYTE PTR [%s], 1\n", regs[a].c_str());
	}
      } else {
	printf("  cmp %s, 0\n", regs[b].c_str());
	printf("  setne %s\n", regs8[b].c_str());
	printf("  movzb %s, %s\n", regs[b].c_str(), regs8[b].c_str());
	printf("  mov [%s], %s\n", regs[a].c_str(), reg(b, ir->type_size).c_str());
      }
    } //if TY_BOOL
    else if(ir->b->isImm){
      const std::string q = sizeQualifier(ir->type_size);
      printf("  mov %s [%s], %d\n", q.c_str(), regs[a].c_str(), ir->b->imm);
    } else {
      printf("  mov [%s], %s\n", regs[a].c_str(), reg(b, ir->type_size).c_str());
    }
    break;
  case IR_STORE_SPILL:
    printf("  mov [rbp-%d], %s\n", ir->lvar->offset, regs[a].c_str());
    break;
  case IR_STORE_ARG:
    printf("  mov [rbp-%d], %s\n", ir->lvar->offset, argreg(ir->imm, ir->type_size).c_str());
    break;
  case IR_LABEL_ADDR:
    printf("  lea %s, %s\n", regs[d].c_str(), ir->name);
    break;
  case IR_BR:
    if(ir->b->isImm){      
      if(ir->b->imm == 0){
	printf("  jmp .L%d\n", ir->bb2->label);
      } else {
	printf("  jmp .L%d\n", ir->bb1->label);
      }
    } else {
      printf("  cmp %s, 0\n", regs[b].c_str());
      printf("  jne .L%d\n", ir->bb1->label);
      printf("  jmp .L%d\n", ir->bb2->label);
    }
    //printf("  jne .L%d\n", ir->bb1->label);
    //printf("  jmp .L%d\n", ir->bb2->label);
    break;
  case IR_JMP:
    if (ir->bbarg){
      if(ir->bbarg->isImm){
	printf("  mov %s, %d\n", regs[ir->bb1->param->rn].c_str(), ir->bbarg->imm);
      } else {
	printf("  mov %s, %s\n", regs[ir->bb1->param->rn].c_str(), regs[ir->bbarg->rn].c_str());
      }
    } //if
    printf("  jmp .L%d\n", ir->bb1->label);
    break;
  case IR_JMP_LABEL:
    printf("  jmp .L.label.%s.%s\n", funcname, ir->dst_label);
    break;
  case IR_LABEL:
    printf(".L.label.%s.%s:\n", funcname, ir->label);
    break;
  case IR_FUNCALL:
    
    for (int i = 0; i < ir->num_args; i++){
      if(ir->args[i]->isImm){
	printf("  mov %s, %d\n", argregs[i].c_str(), ir->args[i]->imm);
      } else {
	printf("  mov %s, %s\n", argregs[i].c_str(), regs[ir->args[i]->rn].c_str());
      }
    } //for

    /*
    const int seq = labelseq++;
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

void emit_data(Program* prog){

  Var* gvar = prog->globals;
  for(gvar; gvar; gvar = gvar->next){
    if(!gvar->is_static){
      printf(".global %s\n", gvar->name);
    } //if
  } //for
  
  printf(".bss\n");

  gvar = prog->globals;
  for(gvar; gvar; gvar = gvar->next){
    if(gvar->is_literal || gvar->initializer){
      continue;
    }
    printf(".align %d\n", gvar->type->align);
    printf("%s:\n", gvar->name);
    printf("  .zero %d\n", gvar->type->size);
  } //for

  printf(".data\n");

  gvar = prog->globals;
  for(gvar; gvar; gvar = gvar->next){
    if(gvar->is_literal){
      printf(".align %d\n", gvar->type->align);
      printf("%s:\n", gvar->name);
      printf("  .string \"%s\"\n", gvar->literal);
    } //if

    if(gvar->initializer){
      printf(".align %d\n", gvar->type->align);
      printf("%s:\n", gvar->name);

      Initializer* init = gvar->initializer;
      for(init; init; init = init->next){
	if(init->label){
	  printf("  .quad %s%+ld\n", init->label, init->addend);
	}else if(init->size == 1){
	  printf("  .byte %d\n", init->val);
	} else {
	  printf("  .%dbyte %d\n", init->size, init->val);
	}
      } //for
    } //if
    
  } //for

} //emit_data()


void emit_text(Program* prog){

  printf(".text\n");

  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    if(!fn->is_static){
      printf(".global %s\n", fn->name);
    } //if
    printf("%s:\n", fn->name);
    funcname = fn->name;

    //プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    //printf("  sub rsp, %d\n", fn->stack_size);
    printf("  sub rsp, %d\n", roundup(fn->stack_size, 16)); //for 16-byte alignment
    printf("  push rbx\n");
    printf("  push r12\n");
    printf("  push r13\n");
    printf("  push r14\n");
    printf("  push r15\n");
    printf("  sub rsp, 8\n"); //for 16-byte alignment
    
    if(fn->has_varargs){
      const int n = fn->params.size();

      printf("mov dword ptr [rbp-8], %d\n", n * 8);
      printf("mov [rbp-16], r9\n");
      printf("mov [rbp-24], r8\n");
      printf("mov [rbp-32], rcx\n");
      printf("mov [rbp-40], rdx\n");
      printf("mov [rbp-48], rsi\n");
      printf("mov [rbp-56], rdi\n");
      
      //must implement for float
      
    } //if varargs
    
    //generate code from IR
    for(auto iter = fn->bbs.begin(), end = fn->bbs.end(); iter != end; ++iter){
      BasicBlock* bb = (*iter);
      printf(".L%d:\n", bb->label);
      for(auto it_inst = bb->instructions.begin(), end_inst = bb->instructions.end();
	  it_inst != end_inst; ++it_inst){
	gen(*it_inst);
      } //for it_inst
    } //for iter

    //エピローグ
    printf(".L.return.%s:\n", funcname);
    printf("  add rsp, 8\n"); //for 16-byte alignment
    printf("  pop r15\n");
    printf("  pop r14\n");
    printf("  pop r13\n");
    printf("  pop r12\n");
    printf("  pop rbx\n");    
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    
} //for fn
  
} //emit_text()

void gen_x86(Program* prog){

  printf(".intel_syntax noprefix\n");

  emit_data(prog);
  emit_text(prog);
  
} //gen_x86()
