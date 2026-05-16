#pragma once

#include <cstdint>

namespace PSQT {

using Score = int32_t;

constexpr Score make_score_const(int mg, int eg) {
    uint32_t low = static_cast<uint32_t>(static_cast<int16_t>(mg)) & 0xFFFFu;
    uint32_t high = static_cast<uint32_t>(static_cast<int16_t>(eg)) << 16;
    return static_cast<Score>(low | high);
}

#define make_score(mg, eg) make_score_const(mg, eg)
#define mg_of(score)     static_cast<Score>(static_cast<int16_t>((score) & 0xFFFF))
#define eg_of(score)     static_cast<Score>(static_cast<int16_t>(((score) >> 16) & 0xFFFF))

enum PhaseIndex : int {
    MG = 0,
    EG = 1,
};

enum PieceType : int {
    PAWN = 0,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    PIECE_TYPE_NB,
};

enum Piece : int {
    W_PAWN = 0,
    W_KNIGHT,
    W_BISHOP,
    W_ROOK,
    W_QUEEN,
    W_KING,
    B_PAWN,
    B_KNIGHT,
    B_BISHOP,
    B_ROOK,
    B_QUEEN,
    B_KING,
    PIECE_NB,
};

enum Square : int {
    SQ_A1 = 0, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQUARE_NB,
};

enum File : int {
    FILE_A = 0,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NB,
};

enum Rank : int {
    RANK_1 = 0,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NB,
};

constexpr Square make_square(File f, Rank r) {
    return Square(int(r) * FILE_NB + int(f));
}

constexpr File file_of(Square s) {
    return File(int(s) & 7);
}

constexpr Rank rank_of(Square s) {
    return Rank(int(s) >> 3);
}

constexpr Square flip_rank(Square s) {
    return Square(int(s) ^ 56);
}

constexpr PieceType piece_type(Piece pc) {
    return PieceType(int(pc) & 7);
}

constexpr Piece mirror_piece(Piece pc) {
    return Piece(int(pc) ^ 6);
}

} // namespace PSQT
