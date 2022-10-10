#ifndef OPTIMIZATION_H
#define OPTIMIZATION_H

#include "../jcc.h"
#include <unordered_map>
#include <string>
#include <utility>
#include <stdio.h>
#include <algorithm>

//local
bool optimize_bb(BasicBlock*);

//util
void constructCFGs(Program*);
void printCFGs(Program*);
void simplifyCFGs(Program*);

//global
void eliminateUnreachableBBs(Program*);

#endif
