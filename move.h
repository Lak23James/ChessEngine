#pragma once
#include <stdint.h>

enum MoveFlags {
    QUIET_MOVE = 0,         // Covers pawn pushes, knight jumps, etc.
    DOUBLE_PAWN_PUSH = 1,
    KING_CASTLE = 2,
    QUEEN_CASTLE = 3,
    CAPTURE = 4,
    EN_PASSANT = 5,
    // Future: PROMOTION_QUEEN = 8, etc.
};

inline uint16_t encode_move(int from, int to, int flags) {
    return (uint16_t)from | (uint16_t)(to << 6) | (uint16_t)(flags << 12);
}

inline int get_move_from(uint16_t move)  { return move & 0x3F; }
inline int get_move_to(uint16_t move)    { return (move >> 6) & 0x3F; }
inline int get_move_flags(uint16_t move) { return (move >> 12) & 0x0F; }