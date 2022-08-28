#include "../jcc.h"
#include <unordered_map>
#include <utility>

bool optimize_bb(BasicBlock*);
bool isBinaryOp(const IRKind opcode);
bool isUnaryOp(const IRKind opcode);
bool constantPropagation_bb(BasicBlock*);
bool peephole(BasicBlock*);
