#include "jcc.h"

//
//register allocator
//
std::unordered_map<int, Reg*> used;
const int num_reg = 7;
//static int spill_reg = 0;

void convertThreeToTwo(){
  //convert d = a op b; --> mov d, a; op d, b;
  //std::list<BasicBlock*> bb;

  for(auto iter = BB_list.begin(), end = BB_list.end(); iter != end; iter++){
    std::list<IR*> vec;
    for(auto it_inst = (*iter)->instructions.begin(), end_inst = (*iter)->instructions.end(); it_inst != end_inst; ++it_inst){
      IR* ir = *it_inst;
      if(!ir->d || !ir->a){
	vec.push_back(*it_inst);
	continue;
      } //if
      
      IR* ir2 = (IR*)calloc(1, sizeof(IR));
      ir2->opcode = IR_MOV;
      ir2->d = ir->d;
      ir2->b = ir->a;
      vec.push_back(ir2);
      
      ir->a = ir->d;
      vec.push_back(ir);
    } //for it_inst
    (*iter)->instructions = vec;
  } //for iter
  //IR_list = vec;
} //convertThreeToTwo

void set_last_use(Reg* r, int ic) {
  if(r && r->last_use < ic){
    r->last_use = ic;
  }
} //set_last_use()


const std::vector<Reg*> collect_reg(){
  std::vector<Reg*> vec;
  int ic = 1; //instruction count

  for(auto iter = BB_list.begin(), end = BB_list.end(); iter != end; ++iter){
    for(auto it_inst = (*iter)->instructions.begin(), end_inst = (*iter)->instructions.end(); it_inst != end_inst; ++it_inst){
      IR* ir = *it_inst;
      if(ir->d && !ir->d->def){
	ir->d->def = ic;
	vec.push_back(ir->d);
      } //if

      set_last_use(ir->a, ic);
      set_last_use(ir->b, ic);
      ic++;
    } //for it_inst
  } //for iter
  return vec;
} //collect_reg()

int choose_to_spill() {
  int k = 0;
  for (int i = 1; i < num_reg; i++)
    if (used[k]->last_use < used[i]->last_use)
      k = i;
  return k;
} //choose_to_spill()

void allocate(const std::vector<Reg*>& vec_reg){
  
  for(int i = 0; i < num_reg; i++){
    used[i] = nullptr;
  } //for
  
  for(auto iter = vec_reg.begin(), end = vec_reg.end(); iter != end; ++iter){
    Reg* reg = *iter;
    bool found = false;
    for(int i = 0; i < num_reg-1; i++){
      if(used[i] && reg->def < used[i]->last_use){
	continue;
      }
      reg->rn = i;
      used[i] = reg;
      found = true;
      break;
    } //for i

    if(found){
      continue;
    } //if

    used[num_reg-1] = reg;
    const int k = choose_to_spill();
    reg->rn = k;
    used[k]->rn = num_reg-1;
    used[k]->spill = true;
    used[k] = reg;
    
  } //for iter

} //allocate

void spill_store(std::list<IR*>& inst_list, IR* ir) {
  Reg* r = ir->d;
  if (!r || !r->spill)
    return;

  IR* ir2 = new IR();
  ir2->opcode = IR_STORE_SPILL;
  ir2->a = r;
  ir2->lvar = r->lvar;
  //vec_push(v, ir2);
  inst_list.push_back(ir2);
} //spill_store()

void spill_load(std::list<IR*>& inst_list, IR* ir, Reg* r) {
  if (!r || !r->spill)
    return;

  IR* ir2 = new IR();
  ir2->opcode = IR_LOAD_SPILL;
  ir2->d = r;
  ir2->lvar = r->lvar;
  //vec_push(v, ir2);
  inst_list.push_back(ir2);
} //spill_load()

void emit_spill_code(BasicBlock* bb) {
  //Vector *v = new_vec();
  std::list<IR*> inst_list;

  for(auto iter = bb->instructions.begin(), end = bb->instructions.end();
       iter != end; ++iter){
    IR* ir = *iter;

    spill_load(inst_list, ir, ir->a);
    spill_load(inst_list, ir, ir->b);
    //spill_load(inst_list, ir, ir->bbarg);
    //vec_push(v, ir);
    inst_list.push_back(ir);
    spill_store(inst_list, ir);
  }
  bb->instructions = inst_list;
} //emit_spill_code()

void allocateRegister(){

  convertThreeToTwo();

  const std::vector<Reg*> vec_reg = collect_reg();
  allocate(vec_reg);

  for(auto iter = vec_reg.begin(), end = vec_reg.end(); iter != end; ++iter){
    Reg* reg = *iter;
    if(!reg->spill){
      continue;
    } //if

    LVar* lvar = new LVar();
    char tmp[] = "spill";
    lvar->name = tmp;
    reg->lvar = lvar;
    if(!locals.empty()){
      lvar->offset = locals.front()->offset + 8;
    } else {
      lvar->offset = 8;
    } //if
        
  } //for iter

  for(auto iter = BB_list.begin(), end = BB_list.end(); iter != end; ++iter){
    emit_spill_code(*iter);
  } //for
  
} //allocateRegister()
