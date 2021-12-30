#include "jcc.h"

Var* locals = NULL;
Var* globals = NULL;
int nlabel = 1;
std::vector<Node*> breaks = {};
std::vector<Node*> continues = {};
std::vector<Node*> switches = {};
//Node* current_switch = nullptr;

//scode for local variable, global variable,
//typedef, and enum constant
typedef struct VarScope VarScope;
struct VarScope{
  VarScope* next;
  char* name;
  int depth;

  Var* var;
  Type* type_def;
  Type* enum_type;
  int enum_val;
};

//scope for struct and enum tags
typedef struct TagScope TagScope;
struct TagScope{
  TagScope* next;
  char* name;
  int depth;
  Type* type;
};

typedef struct Scope Scope;
struct Scope{
  VarScope* var_scope;
  TagScope* tag_scope;
};

VarScope* var_scope = nullptr;
TagScope* tag_scope = nullptr;
int scope_depth = 0;

Scope* enter_scope(){
  Scope* sc = (Scope*)calloc(1, sizeof(Scope));
  sc->var_scope = var_scope;
  sc->tag_scope = tag_scope;
  scope_depth++;
  return sc;
} //enter_scope()

void leave_scope(Scope* sc){
  var_scope = sc->var_scope;
  tag_scope = sc->tag_scope;
  scope_depth--;
} //leave_scope()

VarScope* push_scope(const char* name){
  VarScope* sc = (VarScope*)calloc(1, sizeof(VarScope));
  sc->name = const_cast<char*>(name);
  sc->next = var_scope;
  sc->depth = scope_depth;
  var_scope = sc;
  return sc;
} //push_scope()

void push_tag_scope(Token* tok, Type* type){
  TagScope* sc = (TagScope*)calloc(1, sizeof(TagScope));
  sc->name = strndup(tok->str, tok->len);
  sc->next = tag_scope;
  sc->depth = scope_depth;
  sc->type = type;
  tag_scope = sc;
} //push_tag_scope()

Var* find_lvar(Token* tok){
  Var* var = locals;
  for(var; var; var = var->next){
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
      return var;
    } //if
  } //for
  return NULL;
} //find_lvar()

/*
Var* add_lvar(Type* type, char* name){
  Var* var = (Var*)calloc(1, sizeof(Var));
  var->type = type;
  var->is_local = true;
  var->name = name;
  lvar->next = locals;
  locals = lvar;
  return lvar;
} //add_lvar()
*/

Var* find_gvar(Token* tok){
  Var* var = globals;
  for(var; var; var = var->next){
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)){
      return var;
    } //if
  } //for
  return NULL;
} //find_gvar()

VarScope* find_var_inScope(Token* tok){
  for(VarScope* sc = var_scope; sc; sc = sc->next){
    if(strlen(sc->name) == tok->len && !strncmp(tok->str, sc->name, tok->len)){
      return sc;
    } //if
  } //for
  return nullptr;
} //find_var_inScope()

TagScope* find_tag_inScope(Token* tok){
  for(TagScope* sc = tag_scope; sc; sc = sc->next){
    if(strlen(sc->name) == tok->len && !strncmp(tok->str, sc->name, tok->len)){
      return sc;
    } //for
  } //for
  return nullptr;
} //find_tag_inScope()

Node* new_node(const NodeKind kind){
  Node* node = (Node*)calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node* new_binary(const NodeKind kind, Node* lhs, Node* rhs){
  Node* node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node* new_num(const int val){
  Node *node = new_node(ND_NUM);
  node->val = val;
  node->type = int_type; //とりあえず、int_typeにしておく
  return node;
}

Var* new_var(const char* name, Type* type, const bool is_local){
  Var* var = (Var*)calloc(1, sizeof(Var));
  var->name = name;
  var->len = strlen(name);
  var->type = type;
  var->is_local = is_local;
  return var;
} //new_var()

Node* new_var_node(Var* v){
  Node* node = new_node(ND_VAR);
  node->var = v;
  return node;
} //new_var_node()


//ローカル変数のnew
Var* new_lvar(const char* name, Type* type){
  Var* lvar = new_var(name, type, true);
  VarScope* vs = push_scope(name);
  vs->var = lvar;
  
  lvar->next = locals;
  locals = lvar;
  return lvar;
} //new_lvar

//グローバル変数のnew
Var* new_gvar(char* name, Type* type, const bool is_literal, char* literal){
  Var* gvar = new_var(name, type, false);
  VarScope* vs = push_scope(name);
  vs->var = gvar;
  
  gvar->next = globals;
  gvar->is_literal = is_literal;
  gvar->literal = literal;
  globals = gvar;
  return gvar;
} //new_gvar()

Node* new_expr(const NodeKind kind, Node* e){
  Node* node = new_node(kind);
  node->expr = e;
  return node;
} //new_expr()

Node* new_deref(Var* var){
  return new_unary(ND_DEREF, new_var_node(var));
} //new_deref()

Node* new_stmt_expr(/*const*/ std::vector<Node*>/*&*/ vec){
  Node* last = vec.back();
  vec.pop_back();
  
  std::vector<Node*> v;

  for(auto iter = vec.begin(), end = vec.end(); iter != end; ++iter){
    Node* n = new_expr(ND_EXPR_STMT, *iter);
    add_type(n);
    v.push_back(n);
  } //for

  Node* node = new_node(ND_STMT_EXPR);
  node->stmt = v;
  node->expr = last;
  add_type(node);
  return node;

} //new_stmt_expr()

Initializer* new_init_val(Initializer* cur, const int sz, const int val){
  Initializer* init = (Initializer*)calloc(1, sizeof(Initializer));
  init->size = sz;
  init->val = val;
  cur->next = init;
  return init;
} //new_init_val()

Initializer* new_init_label(Initializer* cur, const char* label, const long addend){
  Initializer* init = (Initializer*)calloc(1, sizeof(Initializer));
  init->label = const_cast<char*>(label);
  init->addend = addend;
  cur->next = init;
  return init;
} //new_init_label()

Initializer* new_init_zero(Initializer* cur, const int nbytes){
  //nbytes分、0埋め
  for(int i = 0; i < nbytes; i++){
    cur = new_init_val(cur, 1, 0);
  } //for
  return cur;
} //new_init_zero()

const bool consume_end() {
  Token *tok = token;
  if(consume("}") || (consume(",") && consume("}"))){
    return true;
  }
  token = tok;
  return false;
} //consume_end()

const bool peek_end() {
  Token *tok = token;
  const bool ret = consume("}") || (consume(",") && consume("}"));
  token = tok;
  return ret;
} //peek_end()

void expect_end() {
  if(!consume_end()){
    expect("}");
  }
} //expect_end()

Initializer* emit_struct_padding(Initializer* cur, Type* parent, Member* mem){

  int start = mem->offset + mem->type->size;
  int end = mem->next ? mem->next->offset : parent->size;
  return new_init_zero(cur, end - start);
  
} //emit_struct_padding()

void skip_excess_elements2() {
  for (;;) {
    if (consume("{")){
      skip_excess_elements2();
    }
    else {
      assign();
    }

    if (consume_end()){
      return;
    }
    expect(",");
  } //for

} //skip_excess_elements2()

void skip_excess_elements() {
  expect(",");
  warn_tok(token, "excess elements in initializer");
  skip_excess_elements2();
} //skip_excess_elements()


//basetype = (builtin-type | struct-decl) "*"* 
//builtin-type = "int" | "char" | "short" | "long" | "void" | "_Bool" | "bool"
Type* basetype(){

  Type* type = nullptr;
  if(consume("int")){
    type = int_type;
  } else if(consume("char")){
    type = char_type;
  } else if(consume("short")){
    type = short_type;
  } else if(consume("long")){
    type = long_type;
  } else if(consume("void")){
    type = void_type;
  } else if(consume("_Bool")){
    type = bool_type;
  } else if(consume("bool")){
    type = bool_type;
  } //if

  if(!peek("int") && !peek("char") && !peek("void")
     && !peek("short") && !peek("long")
     && !peek("_Bool") && !peek("bool")
     ){
    if(peek("struct")){
      type = struct_decl();
    } //if struct
  } //if
  
  while(consume("*")){
    type = pointer_to(type);
  }
  return type;
} //basetype()

//determine whether "global_var" or "function"
bool is_function(){
  Token* t = token;
  bool is_func = false;

  Type* type = basetype();
  if(!consume(";")){
    char* name = expect_ident();
    is_func = name && consume("(");
  }

  token = t;
  return is_func;  
} //is_function()

//gvar_initializer2 = assign
//                    | "{" (gvar_initializer2 ("," gvar_initializer2)* ","? )? "}"
Initializer* gvar_initializer2(Initializer* cur, Type* type){

  Token* tok = token;

  if(type->kind == TY_ARRAY && type->base->kind == TY_CHAR
     && token->kind == TK_STR){
    //in case of char a[]="hoge";
    token = token->next;

    if(type->is_incomplete){
      //要素数が省略されているとき
      type->size = tok->str_len;
      type->array_size = tok->str_len;
      type->is_incomplete = false;
    } //if

    //compare array_size with string_length
    const int len = (type->array_size < tok->str_len)
      ? type->array_size : tok->str_len;

    for(int i = 0; i < len; i++){
      cur = new_init_val(cur, 1, tok->strings[i]);
    } //for
    //type->array_size >= tok->str_len のとき、その差を0埋めする
    return new_init_zero(cur, type->array_size - len);
  } //if TY_ARRAY && TY_CHAR && TK_STR
  

  if(type->kind == TY_ARRAY){
    //in case of TYPE a[] = {hoge, fuga};

    const bool open = consume("{");
    int i = 0;
    //要素数が省略されているとき、何要素でも許容する
    const int limit = type->is_incomplete ? INT_MAX : type->array_size;

    if(!peek("}")){
      do{
	cur = gvar_initializer2(cur, type->base);
	i++;
      } while(i < limit && !peek_end() && consume(","));
    } //if

    if(open && !consume_end()){
      //要素数が超過しているとき
      //T a[3] = {1,2,3,4,5}
      skip_excess_elements();
    } //if

    //set array elements which is not initialized to zero
    //T a[5] = {1,2,3}
    cur = new_init_zero(cur, type->base->size * (type->array_size - i));

    if(type->is_incomplete){
      //要素数が省略されているとき
      type->size = type->base->size * i;
      type->array_size = i;
      type->is_incomplete = false;
    } //if
    return cur;    
  } //if(type->kind == TY_ARRAY)

  
  if(type->kind == TY_STRUCT){
    const bool open = consume("{");
    Member* mem = type->members;

    if(!peek("}")){
      do {
	cur = gvar_initializer2(cur, mem->type);
	cur = emit_struct_padding(cur, type, mem);
	mem = mem->next;
      } while(mem && !peek_end() && consume(","));
    } //if(!peek("}"))

    if(open && !consume_end()){
      skip_excess_elements();
    } //if(open && !consume_end())

    //set excess struct element to zero
    if(mem){
      cur = new_init_zero(cur, type->size - mem->offset);
    } //if(mem)
    
    return cur;
  } //if(type->kind == TY_STRUCT)
  
  
  const bool open = consume("{");
  Node* expression = expr();
  if(open){
    expect_end();
  }

  Var* v = nullptr;
  const long constant = eval2(expression, &v);
  if(v){
    int scale = (v->type->kind == TY_ARRAY)
      ? v->type->base->size : v->type->size;
    return new_init_label(cur, v->name, constant * scale);
  } //if(v)
  
  return new_init_val(cur, type->size, constant);
} //gvar_initializer2()

Initializer* gvar_initializer(Type *type) {
  Initializer head = {};
  gvar_initializer2(&head, type);
  return head.next;
}

//global_var = basetype ident type_suffix ("=" gvar_initializer)? ";"
//           | basetype ";"
void global_var(){
  
  Type* type = basetype();
  if(consume(";")){
    return;
  }
  
  Token* tok = token;
  char* name = expect_ident();
  type = type_suffix(type);

  if(type->kind == TY_VOID){
    error_tok(tok, "variable declared void");
  }
 
  Var* gvar = new_gvar(name, type, false, NULL);

  if(consume("=")){
    gvar->initializer = gvar_initializer(type);
    expect(";");
    return;
  } //if

  if(type->is_incomplete){
    error_tok(tok, "incomplete type");
  }
  expect(";");
  
} //global_var()

// program = (global_var | function)*
Program* program(){
  
  Function head = {};
  Function* curr = &head;

  globals = NULL;

  while(!at_eof()){
    if(is_function()){
      Function* fn = function();
      if(!fn){
	continue;
      }
      curr->next = fn;
      curr = curr->next;
      continue;
    } //if
    global_var();
  } //while()

  Program* prog = (Program*)calloc(1, sizeof(Program));
  prog->globals = globals;
  prog->fns = head.next;
  return prog;
} //program()


//param = basetype ident type_suffix
Var* read_func_param(){

  Type* type = basetype();
  Token* tok = consume_ident();
  type = type_suffix(type);

  if(type->kind == TY_ARRAY){
    type = pointer_to(type->base);
  } //if
  
  if(tok){
    char* name = strndup(tok->str, tok->len);
    return new_lvar(name, type);
  } //if(tok)

  return nullptr;
} //read_func_param()

//params = param ("," param)*
void read_func_params(Function* fn){

  if(consume(")")){
    //引数なしのとき
    return;
  }

  Token* tok = token;
  if(consume("void") && consume(")")){
    //args is void
    return;
  }
  token = tok;

  //fn->params = read_func_param();
  //Var* curr = fn->params;
  std::list<Var*> params;
  params.push_back(read_func_param());
  
  while(!consume(")")){
    expect(",");

    //curr->next = read_func_param();
    //curr = curr->next;
    params.push_back(read_func_param());
  } //while
  fn->params = params;

} //read_func_params()

//function = basetype ident "(" params? ")" ("{" stmt* "}" | ";")
//params = param ("," param)*
//param = basetype ident type_suffix
Function* function(){

  locals = NULL;

  Type* type = basetype();
  //Token* tok = expect_ident();
  char* name = expect_ident();

  //new_gvar(name, type, );
  
  Function* fn = new Function();
  //fn->name = strndup(tok->str, tok->len);
  fn->name = name;
  expect("(");

  Scope* sc = enter_scope();
  read_func_params(fn);

  if(consume(";")){
    leave_scope(sc);
    return nullptr;
  } //if

  //read function body
  Node head = {};
  Node* curr = &head;
  expect("{");
  while(!consume("}")){
    curr->next = stmt();
    curr = curr->next;
  } //while

  leave_scope(sc);
  
  fn->node = head.next;
  fn->locals = locals; 
  return fn;  
} //function()


bool is_typename(){

  return peek("int") || peek("char") || peek("void")
    || peek("_Bool") || peek("bool")
    || peek("short")
    || peek("long")
    || peek("struct");

} //is_typename()

Node* new_desg_node2(Var* var, Designator* desg) {
  if(!desg){
    return new_var_node(var);
  }

  Node* node = new_desg_node2(var, desg->next);
  
  if(desg->mem){
    node = new_unary(ND_MEMBER, node/*, desg->mem->tok*/);
    node->member = desg->mem;
    return node;
  }
  
  node = new_add(node, new_num(desg->index), token);
  return new_unary(ND_DEREF, node);
} //new_desg_node2()

Node* new_desg_node(Var* var, Designator* desg, Node* rhs) {
  Node* lhs = new_desg_node2(var, desg);
  add_type(lhs);
  Node* node = new_binary(ND_ASSIGN, lhs, rhs);
  add_type(node);
  Node* ret = new_expr(ND_EXPR_STMT, node);
  add_type(ret);
  return ret;
} //new_desg_node()

Node* lvar_init_zero(Node* cur, Var* var, Type* ty, Designator* desg) {
  if (ty->kind == TY_ARRAY) {
    for (int i = 0; i < ty->array_size; i++) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, var, ty->base, &desg2);
    }
    return cur;
  }

  cur->next = new_desg_node(var, desg, new_num(0));
  return cur->next;
} //lvar_init_zero()

Node* lvar_initializer2(Node* cur, Var* lvar, Type* type, Designator* desg){

  Token* tok = token;
  
  if(type->kind == TY_ARRAY && type->base->kind == TY_CHAR
     && token->kind == TK_STR){
    //in case of char a[]="hoge";
    token = token->next;

    if(type->is_incomplete){
      type->size = tok->str_len;
      type->array_size = tok->str_len;
      type->is_incomplete = false;
    } //if

    //compare array_size with string_length
    int len = (type->array_size < tok->str_len)
      ? type->array_size : tok->str_len;

    int i = 0;
    for(i; i < len; i++){
      Designator desg2 = {desg, i};
      Node *rhs = new_num(tok->strings[i]);
      cur->next = new_desg_node(lvar, &desg2, rhs);
      cur = cur->next;
    } //for

    //initialize zero
    for (int i = len; i < type->array_size; i++) {
      Designator desg2 = {desg, i};
      cur = lvar_init_zero(cur, lvar, type->base, &desg2);
    }
    return cur;
  } //if TY_ARRAY && TY_CHAR && TK_STR

  if(type->kind == TY_ARRAY){
    bool open = consume("{");
    int i = 0;
    int limit = type->is_incomplete ? INT_MAX : type->array_size;

    if (!peek("}")) {
      do {
        Designator desg2 = {desg, i++};
        cur = lvar_initializer2(cur, lvar, type->base, &desg2);
      } while (i < limit && !peek_end() && consume(","));
    }

    if (open && !consume_end()){
      skip_excess_elements();
    }

    // Set array elements which is not initialized to zero.
    while (i < type->array_size) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, lvar, type->base, &desg2);
    }

    if (type->is_incomplete) {
      type->size = type->base->size * i;
      type->array_size = i;
      type->is_incomplete = false;
    }
    return cur;
  } //if(type->kind == TY_ARRAY)

  if(type->kind == TY_STRUCT){
    const bool open = consume("{");
    Member* mem = type->members;

    if(!peek("}")){
      do {
	Designator desg2 = {desg, 0, mem};
	cur = lvar_initializer2(cur, lvar, mem->type, &desg2);
	mem = mem->next;
      } while(mem && !peek_end() && consume(","));
    } //if(!peek("}"))

    if(open && !consume_end()){
      skip_excess_elements();
    } //if(open && !consume_end())

    //set excess struct elements to zero
    for(; mem; mem = mem->next){
      Designator desg2 = {desg, 0, mem};
      cur = lvar_init_zero(cur, lvar, mem->type, &desg2);
    } //for
    return cur;
  } //if(type->kind == TY_STRUCT)
  
  bool open = consume("{");
  cur->next = new_desg_node(lvar, desg, assign());
  if (open){
    expect_end();
  }
  return cur->next;
  
} //lvar_initializer2()

Node* lvar_initializer(Var* lvar){

  Node head = {};
  lvar_initializer2(&head, lvar, lvar->type, NULL);

  Node* node = new_node(ND_BLOCK);
  node->body = head.next;
  return node;
} //lvar_initializer()


//declaration = basetype ident type_suffix ("=" lvar_initializer)? ";"
//            | basetype ";"
Node* declaration(){
  Token* tok = token;
  Type* type = basetype();

  if(tok = consume(";")){
    return new_node(ND_NULL);
  } //if
  
  char* name = expect_ident();

  type = type_suffix(type);

  if(type->kind == TY_VOID){
    error_tok(tok, "variable declared void");
  }

  Var* lvar = new_lvar(name, type);
  
  if(consume(";")){
    if(type->is_incomplete){
      error_tok(tok, "incomplete type");
    }
    return new_node(ND_NULL); //変数宣言では、コード生成はしない
  } //if

  expect("=");
  Node* node = lvar_initializer(lvar);
  expect(";");
  return node; 
  
} //declaration()


//type_suffix = ("[" const_expr "]" type_suffix)?
Type* type_suffix(Type* type){
  if(!consume("[")){
    return type;
  }

  int size = 0;
  bool is_incomplete = true; //index is omitted?

  if(!consume("]")){
    size = const_expr();
    is_incomplete = false;
    expect("]");
  } //if

  type = type_suffix(type);
  if(type->is_incomplete){
    error("incomplete type");
  }

  type = array_of(type, size);
  type->is_incomplete = is_incomplete;
  return type;
} //type_suffix()

//struct_member = basetype ident type-suffix ";"
Member* struct_member(){
  
  Type* type = basetype();
  Token* tok = token;
  char* name = NULL;
  //type = declarator(type, &name);
  name = expect_ident();
  type = type_suffix(type);
  expect(";");

  Member* mem = (Member*)calloc(1, sizeof(Member));
  mem->name = name;
  mem->type = type;
  mem->tok = tok;
  return mem;
  
} //struct_member()

//struct-decl = "struct" ident? ("{" struct-member "}")?
Type* struct_decl(){

  expect("struct");
  Token* id = consume_ident();
  
  if(id && !peek("{")){
    //process of tag
    TagScope* sc = find_tag_inScope(id);
    
    if(!sc){
      Type* type = struct_type();
      push_tag_scope(id, type);
      return type;
    } //if(!sc)

    if(sc->type->kind != TY_STRUCT){
      error_tok(id, "not a struct tag");
    } //if
    return sc->type;
  } //if(id && !peek("{"))
  

  if(!consume("{")){
    return struct_type(); //incomplete type
  } //if

  Type* type;
  TagScope* sc = nullptr;
  if(id){
    sc = find_tag_inScope(id);
  } //if(id)

  if(sc && sc->depth == scope_depth){
    //If the same tag exists in the same scope,
    //this is a redefinition
    if(sc->type->kind != TY_STRUCT){
      error_tok(id, "not a struct tag");
    }
    type = sc->type;
  } else {
    //register the struct type
    type = struct_type();
    if(id){
      push_tag_scope(id, type);
    } //if
  } //if
  
  //read struct members
  Member head = {};
  Member* cur = &head;

  while(!consume("}")){
    cur->next = struct_member();
    cur = cur->next;
  } //while

  //type = struct_type();
  type->members = head.next;

  //assign offsets within the struct to members
  int offset = 0;
  for(Member* mem = type->members; mem; mem = mem->next){
    if(mem->type->is_incomplete){
      error_tok(mem->tok, "incomplete struct member");
    } //if

    offset = align_to(offset, mem->type->align);
    mem->offset = offset;
    offset += mem->type->size;

    if(type->align < mem->type->align){
      //type->alignをmemberの最大のalignに設定する
      type->align = mem->type->align;
    } //if
  } //for mem

  type->size = align_to(offset, type->align);
  type->is_incomplete = false;
  return type;
  
} //struct_decl()


const long const_expr(){
  return eval(logor());
} //const_expr()

const long eval(Node* node){
  return eval2(node, nullptr);
} //eval()

const long eval2(Node* node, Var** v){
  switch(node->kind){
  case ND_ADD:
    return eval(node->lhs) + eval(node->rhs);
  case ND_PTR_ADD:
    return eval2(node->lhs, v) + eval(node->rhs);
  case ND_SUB:
    return eval(node->lhs) - eval(node->rhs);
  case ND_PTR_SUB:
    return eval2(node->lhs, v) - eval(node->rhs);
  case ND_PTR_DIFF:
    return eval2(node->lhs, v) - eval2(node->rhs, v);
  case ND_MUL:
    return eval(node->lhs) * eval(node->rhs);
  case ND_DIV:
    return eval(node->lhs) / eval(node->rhs);
  case ND_MOD:
    return eval(node->lhs) % eval(node->rhs);
  case ND_BITAND:
    return eval(node->lhs) & eval(node->rhs);
  case ND_BITOR:
    return eval(node->lhs) | eval(node->rhs);
  case ND_BITXOR:
    return eval(node->lhs) ^ eval(node->rhs);
  case ND_BITNOT:
    return ~eval(node->lhs);
  case ND_SHL:
    return eval(node->lhs) << eval(node->rhs);
  case ND_SHR:
    return eval(node->lhs) >> eval(node->rhs);
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_LE:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_NOT:
    return !eval(node->lhs);
  case ND_LOGAND:
    return eval(node->lhs) && eval(node->rhs);
  case ND_LOGOR:
    return eval(node->lhs) || eval(node->rhs);
  case ND_NUM:
    return node->val;
  case ND_ADDR:
    if(!v || *v || node->lhs->kind != ND_VAR || node->lhs->var->is_local){
      error_tok(token, "invalid initializer");
    } //if
    *v = node->lhs->var;
    return 0;
  case ND_VAR:
    if(!v || *v || node->var->type->kind != TY_ARRAY){
      error_tok(token, "invalid initializer");
    } //if
    *v = node->var;
    return 0;
  } //switch()
  
  error("not a constant expression");
} //eval2()


Node* stmt(){
  Node* node = stmt2();
  add_type(node);
  return node;
} //stmt()


//stmt2 = expr ";"
//      | "return" expr? ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "do" stmt "while" "(" expr ")" ";"
//      | "for" "(" (expr? ";" | declaration) expr? ";" expr? ")" stmt
//      | "switch" "(" expr ")" stmt
//      | "case" const-expr ":" stmt
//      | "default" ":" stmt
//      | "{" stmt* "}"
//      | "break" ";"
//      | "continue" ";"
//      | "goto" ident ";"
//      | ident ":" stmt
//      | ";"
//      | declaration
Node* stmt2(){
  Node* node;
  Token* t;

  if(consume("return")){
    if(consume(";")){
      //"return" ";"
      node = new_node(ND_RETURN);
      return node;
    } //if
    // "return" expr ";"
    node = new_node(ND_RETURN);
    node->lhs = expr();
    expect(";");
    return node;
  } //if return

  if(consume("if")){
    //"if" "(" expr ")" stmt ("else" stmt)?
    node = new_node(ND_IF);
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if(consume("else")){
      node->els = stmt();
    }
    return node; 
  } //if if

  if(consume("while")){
    //"while" "(" expr ")" stmt
    node = new_node(ND_WHILE);
    breaks.push_back(node);
    continues.push_back(node);
    
    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    breaks.pop_back();
    continues.pop_back();
    return node;
  } //if "while"

  if(consume("do")){
    node = new_node(ND_DO_WHILE);
    breaks.push_back(node);   
    continues.push_back(node);
    
    node->then = stmt();
    expect("while");
    expect("(");
    node->cond = expr();
    expect(")");
    expect(";");

    breaks.pop_back();   
    continues.pop_back();
    return node;
  } //if(consume("do"))

  if(consume("for")){
    //"for" "(" expr? ";" expr? ";" expr? ")" stmt
    node = new_node(ND_FOR);
    expect("(");
    breaks.push_back(node);
    continues.push_back(node);
    Scope* sc = enter_scope();
    
    //1個目
    if(!consume(";")){
      //先読みして、";"ではなかったとき
      if(is_typename()){
	node->init = declaration();
      } else {
	node->init = expr();
	expect(";");
      }
    } //if(!consume(";"))

    //2個目
    if(!consume(";")){
      node->cond = expr();
      expect(";");
    } //if(!consume(";"))

    //3個目
    if(!consume(")")){
      node->inc = expr();
      expect(")");
    } //if(!consume(")"))
    node->then = stmt();
    
    breaks.pop_back();
    continues.pop_back();
    leave_scope(sc);
    return node;
  } //if "for"

  //switch
  if(consume("switch")){
    node = new_node(ND_SWITCH);
    node->cases = {};
    
    expect("(");
    node->cond = expr();
    expect(")");

    breaks.push_back(node);
    switches.push_back(node);
    //Node* sw = current_switch;
    //current_switch = node;
    
    node->body = stmt();

    //current_switch = sw;
    breaks.pop_back();
    switches.pop_back();
    return node;
  } //if(consume("switch"))

  //case
  if(t = consume("case")){
    if(/*!current_switch*/switches.size() == 0){
      error_tok(t, "stray case");
    }
    int value = const_expr();
    expect(":");

    node = new_node(ND_CASE);
    node->body = stmt();
    node->val = value;
    
    Node* n = switches.back();
    n->cases.push_back(node);
    return node;
  } //if(t = consume("case"))

  //default
  if(t = consume("default")){
    if(/*!current_switch*/switches.size() == 0){
      error_tok(t, "stray default");
    }
    expect(":");

    node = new_node(ND_CASE);
    node->body = stmt();
    
    //current_switch->default_case = node;
    Node* n = switches.back();
    n->_default = node;
    return node;
  } //if(t = consume("default"))

  //block statement
  if(consume("{")){
    //"{" stmt* "}"
    Node head = {};
    Node* curr = &head;
    Scope* sc = enter_scope();

    while(!consume("}")){
      curr->next = stmt();
      curr = curr->next;
    } //while

    leave_scope(sc);
    node = new_node(ND_BLOCK);
    node->body = head.next;
    return node;    
  } //if(consume("{"))

  //"break" ";"
  if(t = consume("break")){
    if(breaks.size() == 0){
      error_tok(t, "stray break");
    }
    expect(";");
    
    node = new_node(ND_BREAK);
    node->target = breaks.back();
    return node;
  } //if(consume("break"))

  //"continue" ";"
  if(t = consume("continue")){
    if(continues.size() == 0){
      error_tok(t, "stray continue");
    }
    expect(";");
    
    node = new_node(ND_CONTINUE);
    node->target = continues.back();
    return node;
  } //if(t = consume("continue"))

  
  //"goto" ident ";"
  if(consume("goto")){
    node = new_node(ND_GOTO);
    node->label_name = expect_ident();
    expect(";");
    return node;
  } //if(consume("goto"))

  //ident ":" stmt
  //labeled statement
  if(t = consume_ident()){
    if(consume(":")){
      Node* node = new_unary(ND_LABEL, stmt());
      node->label_name = strndup(t->str, t->len);
      return node;
    } //if(consume(":"))
    token = t;
  } //if(consume_ident())
  
  // ";"
  if(consume(";")){
    node = new_node(ND_NULL);
    return node;
  } //if(consume(";"))

  if(is_typename()){
    //変数宣言
    return declaration();
  } 

  //expr ";"
  node = expr();
  expect(";");
  return node;
  
} //stmt2()

// expr = assign
Node *expr(){
  return assign();
} //expr()


//a op= b; --> {T* t = &a; *t = *t op b;}
Node* new_assign_eq(NodeKind k, Node* lhs, Node* rhs){
  add_type(lhs);
  add_type(rhs);
  std::vector<Node*> v;

  //T* t = &a;
  Var* var = new_lvar("tmp", pointer_to(lhs->type));
  Node* node = new_binary(ND_ASSIGN, new_var_node(var), new_unary(ND_ADDR, lhs));
  add_type(node);
  v.push_back(node);
  
  //*t = *t op b;
  node = new_binary(ND_ASSIGN,
		    new_deref(var),
		    new_binary(k, new_deref(var), rhs)
		    );
  add_type(node);
  v.push_back(node);

  return new_stmt_expr(v);

} //new_assign_eq()


//assign = logor (assign-op assign)?
//assign-op = "=" | "+=" | "-=" | "*=" | "/=" 
Node* assign(){
  //Node* node = equality();
  Node* node = logor();
  if(consume("=")){
    return new_binary(ND_ASSIGN, node, assign());
  } //if

  if(consume("+=")){
    add_type(node);
    if(node->type->base){
      return new_assign_eq(ND_PTR_ADD, node, assign());
    } else {
      return new_assign_eq(ND_ADD, node, assign());
    }
  } //if +=

  if(consume("-=")){
    add_type(node);
    if(node->type->base){
      return new_assign_eq(ND_PTR_SUB, node, assign());
    } else {
      return new_assign_eq(ND_SUB, node, assign());
    }
  } //if -=

  if(consume("*=")){
    return new_assign_eq(ND_MUL, node, assign());
  }

  if(consume("/=")){
    return new_assign_eq(ND_DIV, node, assign());
  }
  
  return node;
} //assign()

//logor = logand ("||" logand)*
Node* logor(){
  Node* node = logand();
  while(consume("||")){
    node = new_binary(ND_LOGOR, node, logand());
  }  //while
  return node;
} //logor()

//logand = bitor ("&&" bitor)*
Node* logand(){
  Node* node = bit_or();
  while(consume("&&")){
    node = new_binary(ND_LOGAND, node, bit_or());
  }  //while
  return node;
} //logand()

//bitor = birxor ("|" bitxor)*
Node* bit_or(){
  Node* node = bit_xor();
  while(consume("|")){
    node = new_binary(ND_BITOR, node, bit_xor());
  } //while
  return node;
} //bit_or()

//bitxor = bitand ("^" bitand)*
Node* bit_xor(){
  Node* node = bit_and();
  while(consume("^")){
    node = new_binary(ND_BITXOR, node, bit_and());
  } //while
  return node;
} //bitxor()

//bitand = equality ("&" equality)*
Node* bit_and(){
  Node* node = equality();
  while(consume("&")){
    node = new_binary(ND_BITAND, node, equality());
  } //while()
  return node;
} //bitand()

//equality = relatinal ("==" relational | "!=" relational)*
Node* equality(){
  Node* node = relational();

  for(;;){
    if(consume(std::string("==").c_str())){
      node = new_binary(ND_EQ, node, relational());
    } else if(consume(std::string("!=").c_str())){
      node = new_binary(ND_NE, node, relational());
    } else {
      return node;
    } //if
  } //for
} //equality()


//relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*
Node* relational(){
  Node* node = shift();

  for(;;){
    if(consume("<")){
      node = new_binary(ND_LT, node, shift());
    } else if(consume("<=")){
      node = new_binary(ND_LE, node, shift());
    } else if(consume(">")){
      node = new_binary(ND_LT, shift(), node);
    } else if(consume(">=")){
      node = new_binary(ND_LE, shift(), node);
    } else {
      return node;
    }
  } //for
  
} //relational()

// shift = add ("<<" add | ">>" add)*
Node* shift(){
  Node* node = add();
  Token* tok;

  for(;;){
    if(consume("<<")){
      node = new_binary(ND_SHL, node, add());
    }
    else if(consume(">>")){
      node = new_binary(ND_SHR, node, add());
    }
    else{
      return node;
    }
  } //for
  
} //shift()

Node* new_add(Node* lhs, Node* rhs, Token* tok){
  add_type(lhs);
  add_type(rhs);

  if(is_integer(lhs->type) && is_integer(rhs->type)){
    return new_binary(ND_ADD, lhs, rhs);
  }
  if(lhs->type->base && is_integer(rhs->type)){
    //左辺がポインタ 右辺がint
    return new_binary(ND_PTR_ADD, lhs, rhs);
  }
  if(is_integer(lhs->type) && rhs->type->base){
    //左辺がint 右辺がポインタ
    return new_binary(ND_PTR_ADD, rhs, lhs);
  }
  error_tok(tok, "invalid operands");

} //new_add()

Node* new_sub(Node* lhs, Node* rhs, Token* tok){
  add_type(lhs);
  add_type(rhs);

  if(is_integer(lhs->type) && is_integer(rhs->type)){
    return new_binary(ND_SUB, lhs, rhs);
  }
  if(lhs->type->base && is_integer(rhs->type)){
    //左辺がポインタ 右辺がint
    return new_binary(ND_PTR_SUB, lhs, rhs);
  }
  if(lhs->type->base && rhs->type->base){
    //左辺がポインタ 右辺がポインタ
    return new_binary(ND_PTR_DIFF, rhs, lhs);
  }
  error_tok(tok, "invalid operands");

} //new_sub()

//add = mul ("+" mul | "-" mul)*
Node* add(){
  Node* node = mul();
  Token* t;
  
  for(;;){
    if(t = consume("+")){
      //node = new_binary(ND_ADD, node, mul());
      node = new_add(node, mul(), t);
    } else if(t = consume("-")){
      //node = new_binary(ND_SUB, node, mul());
      node = new_sub(node, mul(), t);
    } else {
      return node;
    } //if
  } //for
  
} //add()


// mul = unary ("*" unary | "/" unary | "%" unary)*
Node* mul(){
  Node* node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else if(consume("%"))
      node = new_binary(ND_MOD, node, unary());
    else
      return node;
  } //for
} //mul()

Node* new_unary(NodeKind kind, Node* lhs){
  Node* node = new_node(kind);
  node->lhs = lhs;
  return node;
} //new_unary()

//unary = ("+" | "-" | "*" | "&" | "!" | "~")? unary
//        | ("++" | "--") unary
//        | "sizeof" unary
//        | postfix
Node* unary(){
  if(consume("+")){
    //printf("unary() +\n");
    return unary();
  }
  if(consume("-")){
    return new_binary(ND_SUB, new_num(0), unary());
  }
  if(consume("*")){
    Node* node = new_node(ND_DEREF);
    node->lhs = unary();
    return node;
  }
  if(consume("&")){
    Node* node = new_node(ND_ADDR);
    node->lhs = unary();
    return node;
  }

  if(consume("!")){
    Node* node = new_unary(ND_NOT, unary());
    return node;
  }

  if(consume("~")){
    Node* node = new_unary(ND_BITNOT, unary());
    return node;
  }

  if(consume("++")){
    return new_unary(ND_PRE_INC, unary());
  }
  
  if(consume("--")){
    return new_unary(ND_PRE_DEC, unary());
  }
  
  if(consume("sizeof")){
    Node* node = unary();
    add_type(node);
    return new_num(node->type->size);
  }
  return postfix();

} //urary()


//i++ --> {T* t = &i; T t2 = *t; *t = *t + imm; t2;}
Node* new_post_inc(Node* n, const int imm){
  add_type(n);

  std::vector<Node*> vec;
  Var* t = new_lvar("tmp", pointer_to(n->type));
  Var* t2 = new_lvar("tmp2", n->type);

  //T* t = &i;
  vec.push_back(new_binary(ND_ASSIGN, new_var_node(t), new_unary(ND_ADDR, n)));

  //T t2 = *t;
  vec.push_back(new_binary(ND_ASSIGN, new_var_node(t2), new_deref(t)));
  
  //*t = *t + imm;
  vec.push_back(new_binary(ND_ASSIGN,
			   new_deref(t),
			   new_add(new_deref(t), new_num(imm), token)
			   )
		);

  //t2;
  vec.push_back(new_var_node(t2));
  return new_stmt_expr(vec);
  
} //new_post_inc()

Member* find_member(Type* t, char* name){

  for(Member* mem = t->members; mem; mem = mem->next){
    if(!strcmp(mem->name, name)){
      return mem;
    } //if
  } //for mem

  return NULL;
  
} //find_member()

Node* struct_ref(Node* lhs){
  
  add_type(lhs);
  if(lhs->type->kind != TY_STRUCT){
    //error_tok(lhs->tok, "this is not a struct");
    ;
  } //if

  Token* tok = token;
  Member* mem = find_member(lhs->type, expect_ident());
  if(!mem){
    error_tok(tok, "no such member");
  } //if

  Node* node = new_unary(ND_MEMBER, lhs);
  node->member = mem;
  return node;
  
} //struct_ref()

//postfix = primary ("[" expr "]" | "." ident | "->" ident | "++" | "--")*
Node* postfix(){

  Node* node = primary();
  Token* t;
  
  for(;;){
    if(t = consume("[")){
      //a[b] => *(a + b)
      Node* tmp = new_add(node, expr(), t);
      expect("]");
      node = new_unary(ND_DEREF, tmp);
      continue;
    } //if "["

    if(consume(".")){
      node = struct_ref(node);
      continue;
    } //if "."

    if(consume("->")){
      //a->b is (*a).b
      node = new_unary(ND_DEREF, node);
      node = struct_ref(node);
      continue;
    } //if "->"

    if(consume("++")){
      //node = new_unary(ND_POST_INC, node);
      node = new_post_inc(node, 1);
      continue;
    } //if ++

    if(consume("--")){
      //node = new_unary(ND_POST_DEC, node);
      node = new_post_inc(node, -1);
      continue;
    } //if --

    break;
  } //for

  return node;
} //postfix

Node* func_args(){

  if(consume(")")){
    //引数がないとき
    return nullptr;
  }

  Node* head = assign();
  Node* curr = head;
  while(consume(",")){
    curr->next = assign();
    curr = curr->next;
  } //while
  expect(")");
  return head;

} //func_args()

static char* new_label(){
  static int count = 0;
  char buf[20];
  sprintf(buf, ".L.data.%d", count);
  ++count;
  return strndup(buf, 20);
} //new_label()

// primary = "(" expr ")"
//           | ident func_args?
//           | string_literal
//           | num
Node* primary() {
  if(consume(std::string("(").c_str())) {
    Node *node = expr();
    expect(std::string(")").c_str());
    return node;
  } //if consume("(")

  Token* tok = consume_ident();
  if(tok){
    if(consume("(")){
      //function call
      Node* node = new_node(ND_FUNCALL);
      node->funcname = strndup(tok->str, tok->len); //文字列複製
      node->args = func_args();
      add_type(node);

      //scope check
      
      return node;
    } //if(consume("("))

    //variable
    /*
    Node* node = new_node(ND_VAR);
    Var* lvar = find_lvar(tok);
    Var* gvar = find_gvar(tok);
    
    if(lvar){
      node->var = lvar;
    } else if(gvar){
      node->var = gvar;
    } else {
      error_at(tok->str, "undefined variable");
    } //if
    */
    VarScope* sc = find_var_inScope(tok);
    if(sc){
      if(sc->var){
	return new_var_node(sc->var);
      } //if
    } //if(sc)
    error_at(tok->str, "undefined variable");
    //return node;
  } //if tok

  tok = consume_str();
  if(tok){
    Type* type = array_of(char_type, tok->str_len);
    Var* gvar = new_gvar(new_label(), type, true, tok->strings);
    return new_var_node(gvar);
  } //if(tok) consume_str()

  return new_num(expect_number());
} //primary()

