#pragma once
#include <stdint.h>

enum MoveFlags {
    QUIET_MOVE          = 0,  
    DOUBLE_PAWN_PUSH    = 1,                                      
    KING_CASTLE         = 2, 
    QUEEN_CASTLE        = 3, 
    CAPTURE_MOVE        = 4, 
    EN_PASSANT          = 5, 
    
    PROMOTION_KNIGHT    = 8,  
    PROMOTION_BISHOP    = 9,  
    PROMOTION_ROOK      = 10, 
    PROMOTION_QUEEN     = 11, 
    
    PROMOTION_CAPTURE_KNIGHT = 12,
    PROMOTION_CAPTURE_BISHOP = 13,
    PROMOTION_CAPTURE_ROOK   = 14,
    PROMOTION_CAPTURE_QUEEN  = 15,
};

inline uint16_t encode_move(int from, int to, int flags) {
    return (uint16_t)from | (uint16_t)(to << 6) | (uint16_t)(flags << 12);
}

inline int get_move_from(uint16_t move)  { return move & 0x3F; }
inline int get_move_to(uint16_t move)    { return (move >> 6) & 0x3F; }
inline int get_move_flags(uint16_t move) { return (move >> 12) & 0x0F; }