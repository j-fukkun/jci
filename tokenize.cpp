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
void error_at(char* loc, char* fmt, ...){
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
    return false;
  } //if
  Token* t = token;
  token = token->next;
  return t;
} //consume_ident()

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
    char msg[] = "expented an identifier";
    error_at(token->str, msg);
  } //if
  char* s = strndup(token->str, token->len);
  token = token->next;
  return s;
} //expect_ident()

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
		      "default","do"};
  int i = 0;
  for(i = 0; i < sizeof(kw) / sizeof(*kw); i++){
    const int len = strlen(kw[i]);
    if(startswith(p, kw[i]) && !is_alphabet_or_number(p[len])){
      return kw[i];
    } //if
  } //for
  return NULL;
} //startswith_reserved()

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

    //複数文字を区切る
    //multi-letter
    if(startswith(p, std::string("==").c_str())
       || startswith(p, std::string("!=").c_str())
       || startswith(p, std::string("<=").c_str())
       || startswith(p, std::string(">=").c_str())){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    } //if multi-letter

    //１つの文字を区切る
    //single-letter
    if (strchr("+-*/()<>;=", *p)) {
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
