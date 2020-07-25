#include "jcc.h"

//Type* int_type = &(Type){ TY_INT, nullptr };
//                       kind, size, align
Type* int_type = new Type(TY_INT, 4, 4);
Type* char_type = new Type(TY_CHAR, 1, 1);


bool is_integer(Type* t){
  TypeKind k = t->kind;
  return k == TY_INT || k == TY_CHAR;
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
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_FUNCALL:
  case ND_NUM:
    node->type = int_type;
    return;
  case ND_PTR_ADD:
  case ND_PTR_SUB:
  case ND_ASSIGN:
    node->type = node->lhs->type;
    return;
  case ND_VAR:
    node->type = node->var->type;
    return;
  case ND_ADDR: //address &
    //node->type = pointer_to(node->lhs->type);
    if(node->lhs->type->kind == TY_ARRAY){
      node->type = pointer_to(node->lhs->type->base);
    } else {
      node->type = pointer_to(node->lhs->type);
    }
    return;
  case ND_DEREF: //dereferrence *
    if(!node->lhs->type->base){
      error("invalid pointer dereference");
    } //if

    Type* t = node->lhs->type->base;
    node->type = t;    
    return;
  } //switch()
} //add_type()


