#ifndef EVALUATE_H
#define EVALUATE_H

#include "types.h"

class Board;

namespace PSQT {

void init();
int evaluate(const Board& board);

} // namespace PSQT

#endif