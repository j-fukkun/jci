#include "jcc.h"


/*エラーを報告するための関数*/
/*prnitfと同じ引数を取る*/
void error(char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
void error_at(char* loc, const char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//次のトークンが期待している記号のときには、トークンを一つ読み進めて
//真を返す。それ以外の場合は、偽を返す
bool consume(const char* op){
  if(token->kind != TK_RESERVED
     || strlen(op) != token->len
     || memcmp(token->str, op, token->len))
    {
      return false;
    }
  token = token->next;
  return true;
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
    error_at(token->str, msg, op);
  }
  token = token->next;
} //expect()

/*次のトークンが数値の場合、トークンを一つ読み進めて、その数値を返す。*/
/*それ以外の場合には、エラーを報告する*/
const int expect_number(){
  if (token->kind != TK_NUM){
    char msg[] = "expected a number";
    error_at(token->str, msg);
  }
  int val = token->val;
  token = token->next;
  return val;
} //expect_number()

char* expect_ident(){
  if(token->kind != TK_IDENT){
    char msg[] = "expected an identifier";
    error_at(token->str, msg);
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
		      "default","do","sizeof"};
  int i = 0;
  for(i = 0; i < sizeof(kw) / sizeof(*kw); i++){
    const int len = strlen(kw[i]);
    if(startswith(p, kw[i]) && !is_alphabet_or_number(p[len])){
      return kw[i];
    } //if
  } //for
  return NULL;
} //startswith_reserved()

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
    } //if(*p == '"')

    //複数文字を区切る
    //multi-letter
    if(startswith(p, "==")
       || startswith(p, "!=")
       || startswith(p, "<=")
       || startswith(p, ">=")
       || startswith(p, "++")
       || startswith(p, "--")
       ){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    } //if multi-letter

    //１つの文字を区切る
    //single-letter
    if (strchr("+-*/()<>;={},&[]", *p)) {
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
