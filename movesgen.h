#pragma once

#include <cstdint>
#include "move.h"
#include "bitboard.h"

struct MoveList {
    uint16_t moves[256];
    int count = 0;

    void add_move(uint16_t move) {
        moves[count++] = move;
    }
};

void generate_moves(Board& board, MoveList& list);
uint64_t perft(Board& board, int depth);
