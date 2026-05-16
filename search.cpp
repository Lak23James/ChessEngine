#include "evaluate.h"
#include "movesgen.h"
#include "bitboard.h"
#include "search.h"

using Move = uint16_t;

static inline bool is_king_in_check(const Board& board) {
    int king_square = -1;
    int enemy_side = (board.side_to_move == WHITE) ? BLACK : WHITE;

    if (board.side_to_move == WHITE) {
        uint64_t kings = board.get_white_kings();
        if (kings == 0) return false;
        king_square = __builtin_ctzll(kings);
    } else {
        uint64_t kings = board.get_black_kings();
        if (kings == 0) return false;
        king_square = __builtin_ctzll(kings);
    }

    return board.is_square_attacked(king_square, enemy_side);
}

static MoveList generate_legal_moves(Board& board) {
    MoveList result;
    MoveList all_moves;
    generate_moves(board, all_moves);

    for (int i = 0; i < all_moves.count; ++i) {
        Move move = all_moves.moves[i];
        if (board.make_move(move)) {
            board.unmake_move(move);
            result.add_move(move);
        }
    }
    return result;
}

// Global variable to store the best move found at the root
Move best_move_found = 0;

// The pure Negamax Alpha-Beta function (Returns SCORE)
int alpha_beta(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) {
        int color_multiplier = (board.side_to_move == WHITE) ? 1 : -1;
        return PSQT::evaluate(board) * color_multiplier;
    }

    MoveList moves = generate_legal_moves(board);
    if (moves.count == 0) {
        if (is_king_in_check(board)) {
            return -100000 + depth;
        }
        return 0;
    }

    for (int i = 0; i < moves.count; ++i) {
        Move move = moves.moves[i];
        board.make_move(move);
        int score = -alpha_beta(board, depth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    return alpha;
}

void search_position(Board& board, int depth) {
    int alpha = -500000;
    int beta = 500000;
    
    MoveList moves = generate_legal_moves(board);
    if (moves.count == 0) {
        best_move_found = 0;
        return;
    }

    Move best_move_so_far = moves.moves[0];
    for (int i = 0; i < moves.count; ++i) {
        Move move = moves.moves[i];
        board.make_move(move);
        int score = -alpha_beta(board, depth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score > alpha) {
            alpha = score;
            best_move_so_far = move;
        }
    }

    best_move_found = best_move_so_far;
}

uint16_t get_best_move_found() {
    return best_move_found;
}