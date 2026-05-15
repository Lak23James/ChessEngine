#include "zobrist.h"

uint64_t piece_keys[12][64];
uint64_t enpassant_keys[64];
uint64_t castle_keys[16];
uint64_t side_key;

// Xorshift PRNG state
uint64_t random_state = 1804289383ULL;

// Generate 64-bit pseudo-random numbers
uint64_t get_random_U64() {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 7;
    random_state ^= random_state << 17;
    return random_state;
}

void init_zobrist() {
    // Fill piece keys
    for (int piece = 0; piece < 12; piece++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[piece][square] = get_random_U64();
        }
    }
    // Fill enpassant keys
    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = get_random_U64();
    }
    // Fill castling keys
    for (int i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64();
    }
    // Fill side to move key
    side_key = get_random_U64();
}
