#include "zobrist.h"

#include <random>

uint64_t piece_keys[12][64];
uint64_t enpassant_keys[64];
uint64_t castle_keys[16];
uint64_t side_key;

void init_zobrist() {
    std::mt19937_64 e2(123456789); 
    std::uniform_int_distribution<unsigned long long> dist(0, 0xFFFFFFFFFFFFFFFF);

    // Fill piece keys
    for (int piece = 0; piece < 12; piece++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[piece][square] = dist(e2);
        }
    }
    // Fill enpassant keys
    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = dist(e2);
    }
    // Fill castling keys
    for (int i = 0; i < 16; i++) {
        castle_keys[i] = dist(e2);
    }
    // Fill side to move key
    side_key = dist(e2);
}
