#include "jcc.h"

//
//register allocator
//
void convertThreeToTwo(){
  //convert d = a op b; --> mov d, a; op d, b;
  std::list<IR*> vec;

  for(std::list<IR*>::iterator iter = IR_list.begin(), end = IR_list.end(); iter != end; iter++){
    IR* ir = *iter;
    if(!ir->d || !ir->a){
      vec.push_back(*iter);
      continue;
    } //if

    IR* ir2 = (IR*)calloc(1, sizeof(IR));
    ir2->opcode = IR_MOV;
    ir2->d = ir->d;
    ir2->b = ir->a;
    vec.push_back(ir2);

    ir->a = ir->d;
    vec.push_back(ir);
  } //iter
  IR_list = vec;
} //convertThreeToTwo

const std::vector<Reg*> collect_reg(){
  std::vector<Reg*> vec;

  for(std::list<IR*>::iterator iter = IR_list.begin(), end = IR_list.end(); iter != end; ++iter){
    IR* ir = *iter;
    if(ir->d && !ir->d->def){
      ir->d->def = true;
      vec.push_back(ir->d);
    } //if
  } //for
  return vec;
} //collect_reg()

std::unordered_map<int, Reg*> used;
//std::unordered_map<int, int> mapVtoR;
const int num_reg = 7;

const bool allocateRegister(){

  const std::vector<Reg*> vec_reg = collect_reg();
  
  for(int i = 0; i < num_reg; i++){
    used[i] = nullptr;
  } //for
  
  for(auto iter = vec_reg.begin(), end = vec_reg.end(); iter != end; ++iter){
    Reg* reg = *iter;
    bool found = false;
    for(int i = 0; i < num_reg; i++){
      if(used[i] /*|| mapVtoR[reg->vn]*/){
	continue;
      }
      reg->rn = i;
      used[i] = reg;
      //mapVtoR[reg->vn] = i;
      found = true;
      break;
    } //for i

    /*
    if(found){
      continue;
    } else {
      return false;
    } //if
    */
  } //for iter

  return true;
  
} //allocateRegister
