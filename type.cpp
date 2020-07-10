#include "jcc.h"

//Type* int_type = &(Type){ TY_INT, nullptr };
Type* int_type = new Type(TY_INT);

Type* pointer_to(Type* base){

  Type* type = new Type();
  type->kind = TY_PTR;
  type->base = base;
  return type;

} //pointer_to()
