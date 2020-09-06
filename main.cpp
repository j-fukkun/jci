#include "jcc.h"

char* user_input;
Token* token;
char* filename;


char* read_file(char* path) {
  // Open and read the file.
  FILE* fp = fopen(path, "r");
  if(!fp){
    error("cannot open %s: %s", path, strerror(errno));
  }

  int filemax = 10 * 1024 * 1024;
  char* buf = (char*)malloc(filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if(!feof(fp)){
    error("%s: file too large");
  }

  // Make sure that the string ends with "\n\0".
  if(size == 0 || buf[size - 1] != '\n'){
    buf[size++] = '\n';
  }
  buf[size] = '\0';
  return buf;
} //read_file()

int main(int argc, char **argv){
  if(argc != 2){
    char msg[] = "%s: invalid number of argument";
    error(msg, argv[0]);
  }

  //tokenize
  //user_input = argv[1];
  filename = argv[1];
  user_input = read_file(filename);
  token = tokenize();
  Program* prog = program();

  gen_IR(prog);
  
  //dump_IR(prog);
  
  allocateRegister(prog);

  //スタックサイズを計算
  //すべての変数を、とりあえず、8バイトとする
  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){
    int offset = 0;
    Var* lvar = fn->locals;
    for(lvar; lvar; lvar = lvar->next){
      //offset += 8;
      offset = align_to(offset, lvar->type->align);
      offset += lvar->type->size;
      lvar->offset = offset;
    } //for
    fn->stack_size = offset;
  } //for
  
  gen_x86(prog);
  
  return 0;

} //main
