#include "optimization.h"

static bool isBinaryOp(const IRKind opcode){
  return opcode == IR_ADD
    || opcode == IR_SUB
    || opcode == IR_MUL
    || opcode == IR_DIV
    || opcode == IR_MOD
    || opcode == IR_EQ
    || opcode == IR_NE
    || opcode == IR_LT
    || opcode == IR_LE
    || opcode == IR_PTR_ADD
    || opcode == IR_PTR_SUB
    || opcode == IR_PTR_DIFF
    || opcode == IR_SHL
    || opcode == IR_SHR
    || opcode == IR_BITOR
    || opcode == IR_BITAND
    || opcode == IR_BITXOR;
} //isBinaryOp()

static bool isUnaryOp(const IRKind opcode){
  return opcode == IR_MOV
    //|| opcode == IR_RETURN
    || opcode == IR_STORE
    || opcode == IR_LOAD
    || opcode == IR_BR;
} //isUnaryOp()

static bool constantPropagation_bb(BasicBlock* bb){

  bool changed = false;
  using cons = std::pair<Reg*, int>;
  std::unordered_map<Reg*, int> table = {};
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;

    //generation
    if(ir->opcode == IR_IMM){
      table.insert(cons{ir->d, ir->imm});
    } //if IR_IMM

    if(ir->opcode == IR_MOV && ir->b->isImm){
      table.insert(cons{ir->d, ir->b->imm});
    } //if IR_MOV

    //kill
    if(ir->opcode == IR_CAST){
      table.erase(ir->a);
    }

    //replace
    if(isBinaryOp(ir->opcode)
       && ir->opcode != IR_DIV && ir->opcode != IR_MOD){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
	changed = true;
      }
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	ir->b->isImm = true;
	ir->b->imm = table.find(ir->b)->second;
	changed = true;
      }
    } //if bin-op

    //unary
    if(isUnaryOp(ir->opcode)){
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	ir->b->isImm = true;
	ir->b->imm = table.find(ir->b)->second;
	changed = true;
      }
    } //if unary-ops
    
    if(ir->opcode == IR_RETURN){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a->isImm = true;
	ir->a->imm = table.find(ir->a)->second;
	changed = true;
      }
    } //if RETURN

    if(ir->opcode == IR_JMP){
      if(ir->bbarg){
	if(table.find(ir->bbarg) != table.end() && !ir->bbarg->isImm){
	  ir->bbarg->isImm = true;
	  ir->bbarg->imm = table.find(ir->bbarg)->second;
	  changed = true;
	}
      }
    } //if JMP

    //function call
    if(ir->opcode == IR_FUNCALL){
      for(int i = 0; i < ir->num_args; i++){
	if(table.find(ir->args[i]) != table.end()
	   && !ir->args[i]->isImm){
	  ir->args[i]->isImm = true;
	  ir->args[i]->imm = table.find(ir->args[i])->second;
	  changed = true;
	}
      } //for
    }
    
  } //for iter_inst
  return changed;
} //constantPropagation_bb()

static bool copyPropagation_bb(BasicBlock* bb){

  bool changed = false;
  using P = std::pair<Reg*, Reg*>;
  std::unordered_map<Reg*, Reg*> table = {}; //d --> b
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;

    //generation
    if(ir->opcode == IR_MOV && !ir->b->isImm){
      table.insert(P{ir->d, ir->b});
    } //if IR_MOV

    //kill
    /*
    if(ir->opcode == IR_CAST){
      table.erase(ir->a);
    }
    */

    //replace
    if(isBinaryOp(ir->opcode)){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){	
	ir->a = table.find(ir->a)->second;
	changed = true;
      }
      if(table.find(ir->b) != table.end() && !ir->b->isImm){	
	ir->b = table.find(ir->b)->second;
	changed = true;
      }
    } //if bin-op

    //unary
    if(isUnaryOp(ir->opcode)){
      if(table.find(ir->b) != table.end() && !ir->b->isImm){
	ir->b = table.find(ir->b)->second;
	changed = true;
      }
    } //if unary-ops
    
    if(ir->opcode == IR_RETURN
       || ir->opcode == IR_CAST
       || ir->opcode == IR_STORE){
      if(table.find(ir->a) != table.end() && !ir->a->isImm){
	ir->a = table.find(ir->a)->second;
	changed = true;
      }
    } //if RETURN || CAST || STORE

    if(ir->opcode == IR_JMP){
      if(ir->bbarg){
	if(table.find(ir->bbarg) != table.end() && !ir->bbarg->isImm){
	  ir->bbarg = table.find(ir->bbarg)->second;
	  changed = true;
	}
      }
    } //if JMP

    //function call
    if(ir->opcode == IR_FUNCALL){
      for(int i = 0; i < ir->num_args; i++){
	if(table.find(ir->args[i]) != table.end()
	   && !ir->args[i]->isImm){
	  ir->args[i] = table.find(ir->args[i])->second;
	  changed = true;
	}
      } //for
    } //if FUNCALL
    
  } //for iter_inst
  return changed;
} //copyPropagation_bb()


static bool eliminateRedundantLoadFromStack(BasicBlock* bb){

  bool changed = false;
  using P = std::pair<int, Reg*>;
  std::unordered_map<int, Reg*> mapOffset_Reg = {};
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(ir->opcode == IR_LVAR){
      auto it = mapOffset_Reg.find(ir->lvar->offset);
      if(it != mapOffset_Reg.end()){
	IR* new_ir = new IR();
	new_ir->opcode = IR_MOV;
	new_ir->d = ir->d;
	new_ir->b = it->second;
	new_ir->b->isImm = false;
	iter_inst = bb->instructions.erase(iter_inst);
	iter_inst = bb->instructions.insert(iter_inst, new_ir);	
	changed = true;
      } else {	
	mapOffset_Reg.insert(P{ir->lvar->offset, ir->d});
      } //if
    } //if
    
  } //for iter_inst
  
  return changed;
} //eliminateRedundantLoadFromStack()

static bool eliminateRedundantLoadofGlobalVar(BasicBlock* bb){

  bool changed = false;
  using P = std::pair<std::string, Reg*>;
  std::unordered_map<std::string, Reg*> mapName_Reg = {};
  
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(ir->opcode == IR_LABEL_ADDR){
      auto it = mapName_Reg.find(std::string(ir->name));
      if(it != mapName_Reg.end()){
	IR* new_ir = new IR();
	new_ir->opcode = IR_MOV;
	new_ir->d = ir->d;
	new_ir->b = it->second;
	new_ir->b->isImm = false;
	iter_inst = bb->instructions.erase(iter_inst);
	iter_inst = bb->instructions.insert(iter_inst, new_ir);	
	changed = true;
      } else {	
	mapName_Reg.insert(P{std::string(ir->name), ir->d});
      } //if
    } //if
    
  } //for iter_inst
  
  return changed;
} //eliminateRedundantLoadofGlobalVar()

static bool mem2reg_bb(BasicBlock* bb){

  bool changed = false;
  using P = std::pair<Reg*, Reg*>;
  std::unordered_map<Reg*, Reg*> mapLoad = {}; //ir->b --> ir->d

  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(ir->opcode == IR_LOAD){
      auto it = mapLoad.find(ir->b);
      if(it != mapLoad.end()){
	IR* new_ir = new IR();
	new_ir->opcode = IR_MOV;
	new_ir->d = ir->d;
	new_ir->b = it->second;
	//new_ir->b->isImm = false;
	iter_inst = bb->instructions.erase(iter_inst);
	iter_inst = bb->instructions.insert(iter_inst, new_ir);
	changed = true;
      } else {
	mapLoad.insert(P{ir->b, ir->d});
      }      
    } else if(ir->opcode == IR_STORE){
      mapLoad.clear(); //conservative
      mapLoad.insert(P{ir->a, ir->b});
    } else if(ir->opcode == IR_FUNCALL){
      mapLoad.clear(); //conservative
    }
  } //for iter_inst
  
  return changed;
} //mem2reg_bb()


static IR* createMoveImm(Reg* d, Reg* b, const int imm){
  IR* ir = new IR();
  ir->opcode = IR_MOV;
  ir->d = d;
  ir->b = b;
  ir->b->isImm = true;
  ir->b->imm = imm;
  return ir;
} //createMoveImm()

static IR* createMove(Reg* d, Reg* b){
  IR* ir = new IR();
  ir->opcode = IR_MOV;
  ir->d = d;
  ir->b = b;
  ir->b->isImm = b->isImm;
  ir->b->imm = b->isImm ? b->imm : 0;
  return ir;
} //createMove()

static bool peephole(BasicBlock* bb){

  bool changed = false;
  for(auto iter_inst = bb->instructions.begin(); iter_inst != bb->instructions.end(); ++iter_inst){
    IR* ir = *iter_inst;
    if(isBinaryOp(ir->opcode)
       && ir->a->isImm && ir->b->isImm){
      //constant folding
      if(ir->opcode == IR_ADD){
	const int c = ir->a->imm + ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //ADD
      else if(ir->opcode == IR_SUB){
	const int c = ir->a->imm - ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //SUB
      else if(ir->opcode == IR_MUL){
	const int c = ir->a->imm * ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //MUL
      else if(ir->opcode == IR_DIV){
	const int c = ir->a->imm / ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //DIV
      else if(ir->opcode == IR_MOD){
	const int c = ir->a->imm % ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //MOD
      else if(ir->opcode == IR_EQ){
	const int c = ir->a->imm == ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //EQ
      else if(ir->opcode == IR_NE){
	const int c = ir->a->imm != ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //NE
      else if(ir->opcode == IR_LT){
	const int c = ir->a->imm < ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //LT
      else if(ir->opcode == IR_LE){
	const int c = ir->a->imm <= ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //LE
      else if(ir->opcode == IR_SHL){
	const int c = ir->a->imm << ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //SHL
      else if(ir->opcode == IR_SHR){
	const int c = ir->a->imm >> ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //SHR
      else if(ir->opcode == IR_BITOR){
	const int c = ir->a->imm | ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //BITOR
      else if(ir->opcode == IR_BITAND){
	const int c = ir->a->imm & ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //BITAND
      else if(ir->opcode == IR_BITXOR){
	const int c = ir->a->imm ^ ir->b->imm;
	IR* new_ir = createMoveImm(ir->d, ir->b, c);	  
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      } //BITXOR
    } //if isBinaryOp(ir->opcode) && ir->a->isImm && ir->b->isImm

    //remove unnecessary computation
    if(ir->opcode == IR_ADD){
      IR* new_ir = nullptr;
      if(ir->a->isImm && ir->a->imm == 0){
	//d <= 0 + b is transformed to d <= b
	new_ir = createMove(ir->d, ir->b);	
      } else if(ir->b->isImm && ir->b->imm == 0){
	//d <= a + 0 is transformed to d <= a
	new_ir = createMove(ir->d, ir->a);	
      }
      if(new_ir){
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      }
    } //if(ir->opcode == IR_ADD)
    
    //remove unnecessary computation
    if(ir->opcode == IR_MUL){
      IR* new_ir = nullptr;
      if(ir->a->isImm && ir->a->imm == 1){
	//d <= 1 * b is transformed to d <= b
	new_ir = createMove(ir->d, ir->b);	
      } else if(ir->b->isImm && ir->b->imm == 1){
	//d <= a * 1 is transformed to d <= a
	new_ir = createMove(ir->d, ir->a);	
      }
      if(new_ir){
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      }
    } //if(ir->opcode == IR_MUL)

    //remove unnecessary computation
    if(ir->opcode == IR_SUB){      
      if(ir->b->isImm && ir->b->imm == 0){
	//d <= a - 0 is transformed to d <= a
	IR* new_ir = createMove(ir->d, ir->a);
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      }      
    } //if(ir->opcode == IR_DIV)
    
    //remove unnecessary computation
    if(ir->opcode == IR_DIV){      
      if(ir->b->isImm && ir->b->imm == 1){
	//d <= a / 1 is transformed to d <= a
	IR* new_ir = createMove(ir->d, ir->a);
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      }      
    } //if(ir->opcode == IR_DIV)

    //remove unnecessary computation
    if(ir->opcode == IR_MOD){      
      if(ir->b->isImm && ir->b->imm == 1){
	//d <= a mod 1 is transformed to d <= 0
	IR* new_ir = createMoveImm(ir->d, ir->a, 0);
	auto del_it = bb->instructions.erase(iter_inst);
	auto new_it = bb->instructions.insert(del_it, new_ir);
	iter_inst = new_it;
	changed = true;
      }      
    } //if(ir->opcode == IR_MOD)
    
    if((ir->opcode == IR_JMP && !ir->bbarg) || ir->opcode == IR_JMP_LABEL){
      auto next = std::next(iter_inst, 1);
      if(next != bb->instructions.end()){
	if(((*next)->opcode == IR_JMP && !(*next)->bbarg)
	   || (*next)->opcode == IR_JMP_LABEL){
	  //consecutive jmp instruction
	  auto del_it = bb->instructions.erase(next);	
	  changed = true;
	}
      }
    } //if(ir->opcode == IR_JMP)

    if(ir->opcode == IR_BR && ir->b->isImm){
      //conditional jmp --> unconditional jmp
      BasicBlock* bb = nullptr;
      if(ir->b->imm == 0){
	bb = ir->bb2;
      } else {
	bb = ir->bb1;
      }
      IR* jmp = new IR();
      jmp->opcode = IR_JMP;
      jmp->bb1 = bb;
      jmp->bbarg = nullptr;
      iter_inst = bb->instructions.erase(iter_inst);
      iter_inst = bb->instructions.insert(iter_inst, jmp);
      changed = true;
    } //if(ir->opcode == IR_BR && ir->b->isImm)
      
  } //for iter_inst
  return changed;
} //peephole()

bool optimize_bb(BasicBlock* bb){
  //changed IR --> return true
  //otherwise --> return false
  bool changed = false;
  changed = changed || peephole(bb);
  changed = changed || constantPropagation_bb(bb);
  changed = changed || eliminateRedundantLoadFromStack(bb);
  changed = changed || eliminateRedundantLoadofGlobalVar(bb);
  //changed = changed || copyPropagation_bb(bb);
  changed = changed || mem2reg_bb(bb);
  
  return changed;
} //optimize_bb()
