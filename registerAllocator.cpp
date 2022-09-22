#include "jcc.h"

//
//register allocator
//
std::unordered_map<int, Reg*> used;
const int num_reg = 7;
//static int spill_reg = 0;

void convertThreeToTwo(BasicBlock* bbs){
  //convert d = a op b; --> mov d, a; op d, b;
  
  std::list<IR*> vec;
  for(auto it_inst = bbs->instructions.begin(), end_inst = bbs->instructions.end(); it_inst != end_inst; ++it_inst){
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
  bbs->instructions.clear();
  bbs->instructions.resize(vec.size());
  std::copy(vec.begin(), vec.end(), bbs->instructions.begin());

} //convertThreeToTwo

void set_last_use(Reg* r, int ic) {
  if(r && r->last_use < ic){
    r->last_use = ic;
  }
} //set_last_use()


const std::vector<Reg*> collect_reg(Function* fn){
  std::vector<Reg*> vec;
  int ic = 1; //instruction count
  
  for(auto iter = fn->bbs.begin(), end = fn->bbs.end(); iter != end; ++iter){
    BasicBlock* bb = *iter;
    if(bb->param){
      bb->param->def = ic;
      vec.push_back(bb->param);
      ic++;
    } //if
    
    for(auto it_inst = bb->instructions.begin(), end_inst = bb->instructions.end(); it_inst != end_inst; ++it_inst){
      IR* ir = *it_inst;
      if((ir->d != nullptr) && !ir->d->def){
	ir->d->def = ic;
	vec.push_back(ir->d);
      } //if

      set_last_use(ir->a, ic);
      set_last_use(ir->b, ic);
      set_last_use(ir->bbarg, ic);

      if(ir->opcode == IR_FUNCALL){
        for(int i = 0; i < ir->num_args; i++){
	  set_last_use(ir->args[i], ic);
	} //for
      } //if
      
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
  ir2->lvar = r->lvar; //ir2->lvar = ir->lvar;  
  inst_list.push_back(ir2);
} //spill_store()

void spill_load(std::list<IR*>& inst_list, IR* ir, Reg* r) {
  if (!r || !r->spill)
    return;

  IR* ir2 = new IR();
  ir2->opcode = IR_LOAD_SPILL;
  ir2->d = r;
  ir2->lvar = r->lvar; //ir2->lvar = ir->lvar;
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
    spill_load(inst_list, ir, ir->bbarg);
    inst_list.push_back(ir);
    spill_store(inst_list, ir);
  }
  bb->instructions.clear();
  bb->instructions.resize(inst_list.size());
  std::copy(inst_list.begin(), inst_list.end(), bb->instructions.begin());
} //emit_spill_code()

void allocateRegister(Program* prog){
  Function* fn = prog->fns;
  for(fn; fn; fn = fn->next){

    for(auto iter = fn->bbs.begin(), end = fn->bbs.end();
	iter != end; ++iter){
      convertThreeToTwo(*iter);
    } //for iter

    const std::vector<Reg*> vec_reg = collect_reg(fn);
    allocate(vec_reg);  

    for(auto iter = vec_reg.begin(), end = vec_reg.end(); iter != end; ++iter){
      Reg* reg = *iter;
      if(!reg->spill){
	continue;
      } //if
      
      Var* lvar = new Var();
      char tmp[] = "spill";
      lvar->name = tmp;
      lvar->type = pointer_to(int_type);
      lvar->is_local = true;
      reg->lvar = lvar;
      
      lvar->next = fn->locals;
      fn->locals = lvar;
      
    } //for iter vec_reg
    
    for(auto iter = fn->bbs.begin(), end = fn->bbs.end(); iter != end; ++iter){
      emit_spill_code(*iter);
    } //for
    
  } //for fn
  
} //allocateRegister()
