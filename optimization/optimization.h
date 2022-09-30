#include "../jcc.h"
#include <unordered_map>
#include <string>
#include <utility>

bool optimize_bb(BasicBlock*);
void constructCFG(Function*);

bool isBinaryOp(const IRKind opcode);
bool isUnaryOp(const IRKind opcode);
bool constantPropagation_bb(BasicBlock*);
bool copyPropagation_bb(BasicBlock*);
bool eliminateRedundantLoadFromStack(BasicBlock*);
bool eliminateRedundantLoadofGlobalVar(BasicBlock*);
bool mem2reg_bb(BasicBlock*);
bool peephole(BasicBlock*);
