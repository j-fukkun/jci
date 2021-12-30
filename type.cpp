#include "jcc.h"

//Type* int_type = &(Type){ TY_INT, nullptr };
//                       kind, size, align
Type* char_type = new Type(TY_CHAR, 1, 1);
Type* void_type = new Type(TY_VOID, 1, 1);
Type* bool_type = new Type(TY_BOOL, 1, 1);
Type* int_type = new Type(TY_INT, 4, 4);
Type* short_type = new Type(TY_SHORT, 2, 2);
Type* long_type = new Type(TY_LONG, 8, 8);



bool is_integer(Type* t){
  TypeKind k = t->kind;
  return k == TY_INT
    || k == TY_CHAR
    || k == TY_SHORT
    || k == TY_LONG
    || k == TY_BOOL
    ;
} //is_integer()

Type* new_type(TypeKind kind, int size, int align){
  Type* type = new Type(kind, size, align);
  return type;
} //new_type()

Type* pointer_to(Type* base){
  Type* type = new Type(TY_PTR, 8, 8);
  type->base = base;
  return type;
} //pointer_to()

Type* array_of(Type* base, const int size){
  Type* type = new_type(TY_ARRAY, base->size * size, base->align);
  type->base = base;
  type->array_size = size;
  return type;
} //array_of()

const int align_to(const int n, const int align){
  return (n + align - 1) & ~(align -1);
} //align_to()

Type* struct_type(){
  Type* t = new_type(TY_STRUCT, 0, 1);
  t->is_incomplete = true;
  return t;
} //struct_type()

Type* enum_type(){
  return new_type(TY_ENUM, 4, 4);
} //enum_type()

void add_type(Node *node) {
  if (!node || node->type){
    return;
  }

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);
  add_type(node->expr);
  
  Node* n = node->body;
  for (n; n; n = n->next){
    add_type(n);
  }
  n = node->args;
  for (n; n; n = n->next){
    add_type(n);
  }

  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_PTR_DIFF:
  case ND_MUL:
  case ND_DIV:
  case ND_MOD:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_NOT:
  case ND_LOGOR:
  case ND_LOGAND:
  case ND_FUNCALL:
  case ND_NUM:
  case ND_BITAND:
  case ND_BITOR:
  case ND_BITXOR:
    node->type = int_type;
    return;
  case ND_PTR_ADD:
  case ND_PTR_SUB:
  case ND_ASSIGN:
  case ND_PRE_INC:
  case ND_PRE_DEC:
    //case ND_POST_INC:
    //case ND_POST_DEC:
  case ND_SHL:
  case ND_SHR:
  case ND_BITNOT:
    node->type = node->lhs->type;
    return;
  case ND_VAR:
    node->type = node->var->type;
    return;
  case ND_MEMBER:
    node->type = node->member->type;
    return;
  case ND_ADDR: //address &
    if(node->lhs->type->kind == TY_ARRAY){
      node->type = pointer_to(node->lhs->type->base);
    } else {
      node->type = pointer_to(node->lhs->type);
    }
    return;
  case ND_DEREF: {//dereferrence *
    if(!node->lhs->type->base){
      error("invalid pointer dereference");
    } //if

    Type* t = node->lhs->type->base;
    if(t->kind == TY_VOID){
      error("dereference a  void pointer");
    }
    
    node->type = t;    
    return;
  }
  case ND_STMT_EXPR: {
    node->type = node->expr->type;
    for(auto iter = node->stmt.begin(), end = node->stmt.end();
	iter != end; ++iter){
      add_type(*iter);
    }
    return;
  }
  case ND_EXPR_STMT: {
    node->type = node->expr->type;
    return;
  }
  } //switch()
} //add_type()


