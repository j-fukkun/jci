#include "jcc.h"

char* user_input = NULL;

/*エラーを報告するための関数*/
/*prnitfと同じ引数を取る*/
void error(const char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error message in the following format.
//
// foo.c:10: x = y + 1;
//               ^ <error message here>
void verror_at(char* loc, const char* fmt, va_list ap) {
  // Find a line containing `loc`.
  char *line = loc;
  while (user_input < line && line[-1] != '\n')
    line--;

  char *end = loc;
  while (*end != '\n')
    end++;

  // Get a line number.
  int line_num = 1;
  for (char *p = user_input; p < line; p++)
    if (*p == '\n')
      line_num++;

  // Print out the line.
  int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  // Show the error message.
  int pos = loc - line + indent;
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
}

// Reports an error location and exit.
void error_at(char* loc, const char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
  exit(1);
}

// Reports an error location and exit.
void error_tok(Token* tok, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->str, fmt, ap);
  exit(1);
}

void warn_tok(Token* tok, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->str, fmt, ap);
}


//次のトークンが期待している記号のときには、トークンを一つ読み進めて
//真を返す。それ以外の場合は、偽を返す
Token* consume(const char* op){
  if(token->kind != TK_RESERVED
     || strlen(op) != token->len
     || memcmp(token->str, op, token->len))
    {
      return NULL;
    }
  Token* t = token;
  token = token->next;
  return t;
} //consume()

Token* consume_ident(){
  if(token->kind != TK_IDENT){
    return nullptr;
  } //if
  Token* t = token;
  token = token->next;
  return t;
} //consume_ident()

Token* consume_str(){
  if(token->kind != TK_STR){
    return NULL;
  } //if
  Token* t = token;
  token = token->next;
  return t;  
} //consume_str()

/*次のトークンが期待している記号のときには、トークンを一つ読み進める。*/
/*それ以外の場合には、エラーを報告する*/
void expect(const char* op){
  if (token->kind != TK_RESERVED
      || strlen(op) != token->len
      || memcmp(token->str, op, token->len)){
    char msg[] = "expected '%c'";
    error_tok(token, msg, op);
  }
  token = token->next;
} //expect()

/*次のトークンが数値の場合、トークンを一つ読み進めて、その数値を返す。*/
/*それ以外の場合には、エラーを報告する*/
const int expect_number(){
  if (token->kind != TK_NUM){
    char msg[] = "expected a number";
    error_tok(token, msg);
  }
  int val = token->val;
  token = token->next;
  return val;
} //expect_number()

char* expect_ident(){
  if(token->kind != TK_IDENT){
    char msg[] = "expected an identifier";
    error_tok(token, msg);
  } //if
  char* s = strndup(token->str, token->len);
  //Token* t = token;
  token = token->next;
  return s;
  //return t;
} //expect_ident()

Token* peek(const char* s){
  if(token->kind != TK_RESERVED
     || strlen(s) != token->len
     || strncmp(token->str, s, token->len)){
    return NULL;
  }
  return token;
} //peek()

const bool at_eof(){
  return token->kind == TK_EOF;
} //at_eof()

/*新しいトークンを作成してcurにつなげる*/
Token* new_token(TokenKind kind, Token* cur, char* str, int len){
  Token* tok = (Token*)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len  = len;
  cur->next = tok;
  return tok;
} //new_token()

const bool is_alphabet(char c){
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
} //is_alphabet()

const bool is_alphabet_or_number(char c){
  return is_alphabet(c) || ('0' <= c && c <= '9');
} //is_alphabet_or_number()

const bool startswith(char* p, const char* q){
  return (memcmp(p, q, strlen(q)) == 0);
} //startswith()

const char* startswith_reserved(char* p){
  //keyword
  const char* kw[] = {"return","if","else","while","for",
		      "int","char","short","long","void",
		      "break","continue","switch","case","goto",
		      "default","do","sizeof","struct", "enum",
		      "_Bool", "bool", "typedef", "static", "extern",
		      "signed"
  };
  int i = 0;
  for(i = 0; i < sizeof(kw) / sizeof(*kw); i++){
    const int len = strlen(kw[i]);
    if(startswith(p, kw[i]) && !is_alphabet_or_number(p[len])){
      return kw[i];
    } //if
  } //for
  return NULL;
} //startswith_reserved()

char get_escape_char(char c){
  
  switch(c){
  case 'a': return '\a';
  case 'b': return '\b';
  case 't': return '\t';
  case 'n': return '\n';
  case 'v': return '\v';
  case 'f': return '\f';
  case 'r': return '\r';
  case 'e': return 27;
  case '0': return 0;
  default: return c;
  } //switch
  
} //get_escape_char()

Token* read_char_literal(Token* cur, char* start){

  char* p = start + 1;
  if(*p == '\0'){
    error_at(start, "unclosed character literal");
  } //if

  char c;
  if(*p == '\\'){
    p++;
    c = get_escape_char(*p++);
  } else {
    c = *p++;
  } //if

  if(*p != '\''){
    error_at(start, "character literal is too long");
  } //if

  p++;
  Token* tok = new_token(TK_NUM, cur, start, p - start);
  tok->val = c;
  //tok->type = int_type;
  return tok;
  
} //read_char_literal()

Token* read_string(Token* cur, char* start){

  char* p = start + 1;
  char buf[1024];
  int len = 0;

  for(;;){
    if(len == sizeof(buf)){
      error_at(start, "string literal is too large");
    }
    if(*p == '\0'){
      error_at(start, "unclosed string literal");
    }
    if(*p == '"'){
      break;
    }
    /*
    if(*p == '\\'){
      p++;
      buf[len++] = get_escape_char(*p++);
    } else {
      buf[len++] = *p++;
    } //if
    */

    if(*p == '\\' && *(p+1) == '\"'){
      buf[len++] = '\\';
      ++p;
      buf[len++] = '\"';
      ++p;
      continue;
    } //if
    
    buf[len] = *p;
    ++len;
    ++p;
    
  } //for

  Token* tok = new_token(TK_STR, cur, start, p-start+1);
  tok->strings = (char*)malloc(len + 1);
  memcpy(tok->strings, buf, len);
  tok->strings[len] = '\0'; //文字列の最後
  tok->str_len = len + 1;
  return tok;

} //read_string()

/*入力文字列pをトークナイズしてそれを返す*/
Token* tokenize(){
  char* p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    /*空白文字をスキップ*/
    if (isspace(*p)) {
      p++;
      continue;
    }

    //skip comment
    if(startswith(p, "//")){
      p += 2;
      while(*p != '\n'){
	p++;
      }
      continue;
    } //if(startswith(p, "//"))

    if(startswith(p, "/*")){
      char* q = strstr(p+2, "*/");
      //strstrは、文字列から文字列を探して、渡された引数が
      //見つかったとき、その先頭へのポインタを返す
      //見つからなかったときはNULLを返す
      if(!q){
	error_at(p, "unclosed block comment");
      }
      p = q + 2; //pを*/の直後にセット
      continue;
    } //if(startswith(p, "/*"))

    //string literal
    if(*p == '"'){
      cur = read_string(cur, p);
      p += cur->len;
      continue;
    } //if(*p == '"')

    //character literal
    if(*p == '\''){
      cur = read_char_literal(cur, p);
      p += cur->len;
      continue;
    } //if(*p == '\'')

    //複数文字を区切る
    //multi-letter
    //3-letter
    if(startswith(p, "<<=")
       || startswith(p, ">>=")
       || startswith(p, "...")
       ){
      cur = new_token(TK_RESERVED, cur, p, 3);
      p += 3;
      continue;
    } //if multi-letter 3-letter
    
    //multi-letter 2-letter
    if(startswith(p, "==")
       || startswith(p, "!=")
       || startswith(p, "<=")
       || startswith(p, ">=")
       || startswith(p, "++")
       || startswith(p, "--")
       || startswith(p, "||")
       || startswith(p, "&&")
       || startswith(p, "<<")
       || startswith(p, ">>")
       || startswith(p, "+=")
       || startswith(p, "-=")
       || startswith(p, "*=")
       || startswith(p, "/=")
       || startswith(p, "->")
       || startswith(p, "&=")
       || startswith(p, "|=")
       || startswith(p, "^=")
       ){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    } //if multi-letter 2-letter


    //１つの文字を区切る
    //single-letter
    if (strchr("+-*/()<>;={},&[].:!|^~%?", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    } //if single-letter

    //integer
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char* q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    //for keywords
    const char* kw = startswith_reserved(p);
    if(kw){
      const int len = strlen(kw);
      cur = new_token(TK_RESERVED, cur, p, len);
      p += len;
      continue;
    } //if

    //identifier
    if(is_alphabet(*p)){
      char* q = p++;
      while(is_alphabet_or_number(*p)){
	p++;
      } //while
      cur = new_token(TK_IDENT, cur, q, p-q);
      continue;
    } //if is_alphabet

    char msg[] = "invalid token";
    error_at(p, msg);
  } //while()

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// Removes backslashes followed by a newline.
static void remove_backslash_newline(char *p) {
  int i = 0, j = 0;

  // We want to keep the number of newline characters so that
  // the logical line number matches the physical one.
  // This counter maintain the number of newlines we have removed.
  int n = 0;

  while (p[i]) {
    if (p[i] == '\\' && p[i + 1] == '\n') {
      i += 2;
      n++;
    } else if (p[i] == '\n') {
      p[j++] = p[i++];
      for (; n > 0; n--)
        p[j++] = '\n';
    } else {
      p[j++] = p[i++];
    }
  }

  for (; n > 0; n--)
    p[j++] = '\n';
  p[j] = '\0';
  
} //remove_backslash_newline

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

Token* tokenize_file(char* filename){

  user_input = read_file(filename);
  if(!user_input) return NULL;

  remove_backslash_newline(user_input);
  
  return tokenize();
  
} //tokenize_file()
