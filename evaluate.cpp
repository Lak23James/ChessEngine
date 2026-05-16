#include "evaluate.h"
#include "bitboard.h"
#include <algorithm>

namespace PSQT {

static inline int popcount(uint64_t x) {
    return __builtin_popcountll(x);
}

static inline int pop_lsb(uint64_t& bits) {
    int square = __builtin_ctzll(bits);
    bits &= bits - 1;
    return square;
}

constexpr Score PieceValue[2][PIECE_NB] = {
    { 82, 337, 365, 477, 1025, 0, 82, 337, 365, 477, 1025, 0 },
    { 94, 281, 297, 512, 936, 0, 94, 281, 297, 512, 936, 0 }
};

#define S(mg, eg) make_score(mg, eg)

constexpr Score Bonus[PIECE_TYPE_NB][RANK_NB][FILE_NB / 2] = {
    { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
    { // Knight
        { S(-175, -96), S(-92, -65), S(-74, -49), S(-73, -21) },
        { S( -77, -67), S(-41, -54), S(-27, -18), S(-15,   8) },
        { S( -61, -40), S(-17, -27), S(   6,  -8), S(  12,  29) },
        { S( -35, -35), S(   8,  -2), S(  40,  13), S(  49,  28) },
        { S( -34, -45), S(  13, -16), S(  44,   9), S(  51,  39) },
        { S(  -9, -51), S(  22, -44), S(  58, -16), S(  53,  17) },
        { S( -67, -69), S( -27, -50), S(   4, -51), S(  37,  12) },
        { S(-201,-100), S( -83, -88), S( -56, -56), S( -26, -17) }
    },
    { // Bishop
        { S( -53, -57), S(  -5, -30), S(  -8, -37), S( -23, -12) },
        { S( -15, -37), S(   8, -13), S(  19, -17), S(   4,   1) },
        { S(  -7, -16), S(  21,  -1), S(  -5,  -2), S(  17,  10) },
        { S(  -5, -20), S(  11,  -6), S(  25,   0), S(  39,  17) },
        { S( -12, -17), S(  29,  -1), S(  22, -14), S(  31,  15) },
        { S( -16, -30), S(   6,   6), S(   1,   4), S(  11,   6) },
        { S( -17, -31), S( -14, -20), S(   5,  -1), S(   0,   1) },
        { S( -48, -46), S(   1, -42), S( -14, -37), S( -23, -24) }
    },
    { // Rook
        { S( -31,  -9), S( -20, -13), S( -14, -10), S(  -5,  -9) },
        { S( -21, -12), S( -13,  -9), S(  -8,  -1), S(   6,  -2) },
        { S( -25,   6), S( -11,  -8), S(  -1,  -2), S(   3,  -6) },
        { S( -13,  -6), S(  -5,   1), S(  -4,  -9), S(  -6,   7) },
        { S( -27,  -5), S( -15,   8), S(  -4,   7), S(   3,  -6) },
        { S( -22,   6), S(  -2,   1), S(   6,  -7), S(  12,  10) },
        { S(  -2,   4), S(  12,   5), S(  16,  20), S(  18,  -5) },
        { S( -17,  18), S( -19,   0), S(  -1,  19), S(   9,  13) }
    },
    { // Queen
        { S(   3, -69), S(  -5, -57), S(  -5, -47), S(   4, -26) },
        { S(  -3, -55), S(   5, -31), S(   8, -22), S(  12,  -4) },
        { S(  -3, -39), S(   6, -18), S(  13,  -9), S(   7,   3) },
        { S(   4, -23), S(   5,  -3), S(   9,  13), S(   8,  24) },
        { S(   0, -29), S(  14,  -6), S(  12,   9), S(   5,  21) },
        { S(  -4, -38), S(  10, -18), S(   6, -12), S(   8,   1) },
        { S(  -5, -50), S(   6, -27), S(  10, -24), S(   8,  -8) },
        { S(  -2, -75), S(  -2, -52), S(   1, -43), S(  -2, -36) }
    },
    { // King
        { S( 271,   1), S( 327,  45), S( 271,  85), S( 198,  76) },
        { S( 278,  53), S( 303, 100), S( 234, 133), S( 179, 135) },
        { S( 195,  88), S( 258, 130), S( 169, 169), S( 120, 175) },
        { S( 164, 103), S( 190, 156), S( 138, 172), S(  98, 172) },
        { S( 154,  96), S( 179, 166), S( 105, 199), S(  70, 199) },
        { S( 123,  92), S( 145, 172), S(  81, 184), S(  31, 191) },
        { S(  88,  47), S( 120, 121), S(  65, 116), S(  33, 131) },
        { S(  59,  11), S(  89,  59), S(  45,   73), S(  -1,  78) }
    }
};

constexpr Score PBonus[RANK_NB][FILE_NB] = {
    { S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0) },
    { S(   3, -10), S(   3,  -6), S(  10,  10), S(  19,   0), S(  16,  14), S(  19,   7), S(   7,  -5), S(  -5, -19) },
    { S(  -9, -10), S( -15, -10), S(  11, -10), S(  15,   4), S(  32,   4), S(  22,   3), S(   5,  -6), S( -22,  -4) },
    { S(  -4,   6), S( -23,  -2), S(   6,  -8), S(  20,  -4), S(  40, -13), S(  17, -12), S(   4, -10), S(  -8,  -9) },
    { S(  13,  10), S(   0,   5), S( -13,   4), S(   1,  -5), S(  11,  -5), S(  -2,  -5), S( -13,  14), S(   5,   9) },
    { S(   5,  28), S( -12,  20), S(  -7,  21), S(  22,  28), S(  -8,  30), S(  -5,   7), S( -15,   6), S(  -8,  13) },
    { S(  -7,   0), S(   7, -11), S(  -3,  12), S( -13,  21), S(   5,  25), S( -16,  19), S(  10,   4), S(  -8,   7) },
    { S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0), S(   0,   0) }
};

#undef S

Score psq[PIECE_NB][SQUARE_NB];

static constexpr int PhaseValue[PIECE_NB] = {
    0, 1, 1, 2, 4, 0,
    0, 1, 1, 2, 4, 0
};

void init() {
    for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING }) {
        Score value = make_score(PieceValue[MG][pc], PieceValue[EG][pc]);
        PieceType type = piece_type(pc);

        for (int s = 0; s < SQUARE_NB; ++s) {
            Square square = Square(s);
            Score bonus = 0;

            if (type == PAWN) {
                bonus = PBonus[rank_of(square)][file_of(square)];
            } else {
                bonus = Bonus[type][rank_of(square)][file_of(square) / 2];
            }

            psq[pc][square] = value + bonus;
            psq[mirror_piece(pc)][flip_rank(square)] = Score(-psq[pc][square]);
        }
    }
}

int calculate_phase(const Board& board) {
    int phase = 0;
    phase += PhaseValue[W_KNIGHT] * (popcount(board.get_white_knights()) + popcount(board.get_black_knights()));
    phase += PhaseValue[W_BISHOP] * (popcount(board.get_white_bishops()) + popcount(board.get_black_bishops()));
    phase += PhaseValue[W_ROOK]   * (popcount(board.get_white_rooks())   + popcount(board.get_black_rooks()));
    phase += PhaseValue[W_QUEEN]  * (popcount(board.get_white_queens())  + popcount(board.get_black_queens()));
    return std::min(phase, 24);
}

int evaluate(const Board& board) {
    int mg_score = 0;
    int eg_score = 0;

    auto accumulate = [&](uint64_t bits, Piece piece) {
        while (bits) {
            int square = pop_lsb(bits);
            Score packed = psq[piece][Square(square)];
            mg_score += mg_of(packed);
            eg_score += eg_of(packed);
        }
    };

    accumulate(board.get_white_pawns(), W_PAWN);
    accumulate(board.get_white_knights(), W_KNIGHT);
    accumulate(board.get_white_bishops(), W_BISHOP);
    accumulate(board.get_white_rooks(), W_ROOK);
    accumulate(board.get_white_queens(), W_QUEEN);
    accumulate(board.get_white_kings(), W_KING);

    accumulate(board.get_black_pawns(), B_PAWN);
    accumulate(board.get_black_knights(), B_KNIGHT);
    accumulate(board.get_black_bishops(), B_BISHOP);
    accumulate(board.get_black_rooks(), B_ROOK);
    accumulate(board.get_black_queens(), B_QUEEN);
    accumulate(board.get_black_kings(), B_KING);

    int phase = calculate_phase(board);
    int total = (mg_score * phase + eg_score * (24 - phase)) / 24;
    return total;
}

} // namespace PSQT
