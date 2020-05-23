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
}

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
}

const bool at_eof(){
  return token->kind == TK_EOF;
}

/*新しいトークンを作成してcurにつなげる*/
Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
  Token* tok = (Token*)calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len  = len;
  cur->next = tok;
  return tok;
}

const bool startswith(char* p, const char* q){
  return (memcmp(p, q, strlen(q)) == 0);
}

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
    if (strchr("+-*/()<>", *p)) {
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

    char msg[] = "invalid token";
    error_at(p, msg);
  } //while()

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
