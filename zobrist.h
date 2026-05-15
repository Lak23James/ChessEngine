#pragma once
#include <cstdint>

// Extern definitions so the arrays can be accessed anywhere
extern uint64_t piece_keys[12][64];
extern uint64_t enpassant_keys[64];
extern uint64_t castle_keys[16];
extern uint64_t side_key;

// Function to initialize the random keys
void init_zobrist();
