#include "jcc.h"

//char* user_input;
Token* token;
char* filename;

int main(int argc, char **argv){
  if(argc != 2){
    char msg[] = "%s: invalid number of argument";
    error(msg, argv[0]);
  }

  //tokenize
  //user_input = argv[1];
  filename = argv[1];
  //user_input = read_file(filename);
  token = tokenize_file(filename);
  Program* prog = program();

  gen_IR(prog);

  //calculate the size of stack flame
  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    int offset = 0;
    Var* lvar = fn->locals;
    for(lvar; lvar; lvar = lvar->next){      
      offset = align_to(offset, lvar->type->align);
      offset += lvar->type->size;
      lvar->offset = offset;
    } //for
    fn->stack_size = offset;
  } //for

  std::string filename_str = filename;
  dump_IR(prog, std::string(filename_str + ".lir"));
  
  optimize(prog);

  dump_IR(prog, std::string(filename_str + "_optimized.lir"));
  
  allocateRegister(prog);

  //re-calculate the size of stack flame for spilled registers
  /*Function**/ fn = prog->fns;
  for(fn; fn; fn = fn->next){
    int offset = fn->stack_size;
    Var* lvar = fn->locals;
    for(lvar; lvar; lvar = lvar->next){
      if(lvar->offset != 0)
	continue;
      offset = align_to(offset, lvar->type->align);
      offset += lvar->type->size;
      lvar->offset = offset;
    } //for
    fn->stack_size = offset;
  } //for

  dump_IR(prog, std::string(filename_str + "_allocated.lir"));
  
  gen_x86(prog);
  
  return 0;

} //main
