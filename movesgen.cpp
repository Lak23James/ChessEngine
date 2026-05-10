#include "move.h"
#include "bitboard.h"

// Container for all generated moves
struct MoveList {
    uint16_t moves[256];
    int count = 0;

    void add_move(uint16_t move) {
        moves[count++] = move;
    }
};

void promotions(Board& board, MoveList& list, int side_to_move, int from_square, int to_square, bool is_capture) {
    if (side_to_move == WHITE && (from_square / 8 == 6)) {
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_QUEEN : PROMOTION_QUEEN));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_ROOK : PROMOTION_ROOK));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_BISHOP : PROMOTION_BISHOP));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_KNIGHT : PROMOTION_KNIGHT));
    } else if (side_to_move == BLACK && (from_square / 8 == 1)) {
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_QUEEN : PROMOTION_QUEEN));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_ROOK : PROMOTION_ROOK));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_BISHOP : PROMOTION_BISHOP));
        list.add_move(encode_move(from_square, to_square, is_capture ? PROMOTION_CAPTURE_KNIGHT : PROMOTION_KNIGHT));
    }
}

void generate_moves(Board& board, MoveList& list) {
    uint64_t white_pieces = board.get_white_pieces();
    uint64_t black_pieces = board.get_black_pieces();
    uint64_t empty_squares = board.get_empty();
    uint64_t all_occupied = board.get_occupied();

    if (board.side_to_move == WHITE) {
        // --- WHITE PAWNS ---
        uint64_t white_pawns = board.get_white_pawns();
        uint64_t WPtargets = (white_pawns << 8) & empty_squares;
        uint64_t WPtargets2 = (WPtargets << 8) & empty_squares & RANK_4;

        while (WPtargets) {
            int to_square = __builtin_ctzll(WPtargets);
            int from_square = to_square - 8;
            if (from_square / 8 == 6) {
                promotions(board, list, WHITE, from_square, to_square, false);
            } else {
                list.add_move(encode_move(from_square, to_square, QUIET_MOVE));
            }
            WPtargets &= (WPtargets - 1);
        }
        while (WPtargets2) {
            int to_square = __builtin_ctzll(WPtargets2);
            list.add_move(encode_move(to_square - 16, to_square, DOUBLE_PAWN_PUSH));
            WPtargets2 &= (WPtargets2 - 1);
        }

        uint64_t current_white_pawns = white_pawns;
        while (current_white_pawns) {
            int from_square = __builtin_ctzll(current_white_pawns);
            uint64_t valid_captures = board.pawn_attacks_white[from_square] & black_pieces;
            while (valid_captures) {
                int to_square = __builtin_ctzll(valid_captures);
                if (from_square / 8 == 6) {
                    promotions(board, list, WHITE, from_square, to_square, true);
                } else {
                    list.add_move(encode_move(from_square, to_square, CAPTURE_MOVE));
                }
                valid_captures &= (valid_captures - 1);
            }
            current_white_pawns &= (current_white_pawns - 1);
        }

        // --- WHITE KNIGHTS ---
        uint64_t white_knights = board.get_white_knights();
        while (white_knights) {
            int from_square = __builtin_ctzll(white_knights);
            uint64_t valid_moves = board.knight_attacks[from_square] & ~white_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            white_knights &= (white_knights - 1);
        }

        // --- WHITE KINGS ---
        uint64_t white_kings = board.get_white_kings();
        while (white_kings) {
            int from_square = __builtin_ctzll(white_kings);
            uint64_t valid_moves = board.king_attacks[from_square] & ~white_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            white_kings &= (white_kings - 1);
        }

        // --- WHITE ROOKS ---
        uint64_t white_rooks = board.get_white_rooks();
        while (white_rooks) {
            int from_square = __builtin_ctzll(white_rooks);
            board.init_rooks(all_occupied, from_square);
            uint64_t valid_moves = board.rook_attacks[from_square] & ~white_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            white_rooks &= (white_rooks - 1);
        }

        // --- WHITE BISHOPS ---
        uint64_t white_bishops = board.get_white_bishops();
        while (white_bishops) {
            int from_square = __builtin_ctzll(white_bishops);
            board.init_bishops(all_occupied, from_square);
            uint64_t valid_moves = board.bishop_attacks[from_square] & ~white_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            white_bishops &= (white_bishops - 1);
        }

        // --- WHITE QUEENS ---
        uint64_t white_queens = board.get_white_queens();
        while (white_queens) {
            int from_square = __builtin_ctzll(white_queens);
            board.init_queens(all_occupied, from_square);
            uint64_t valid_moves = board.queen_attacks[from_square] & ~white_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            white_queens &= (white_queens - 1);
        }

    } else { // BLACK TO MOVE
        // --- BLACK PAWNS ---
        uint64_t black_pawns = board.get_black_pawns();
        uint64_t BPtargets = (black_pawns >> 8) & empty_squares;
        uint64_t BPtargets2 = (BPtargets >> 8) & empty_squares & RANK_5;

        while (BPtargets) {
            int to_square = __builtin_ctzll(BPtargets);
            int from_square = to_square + 8;
            if (from_square / 8 == 1) {
                promotions(board, list, BLACK, from_square, to_square, false);
            } else {
                list.add_move(encode_move(from_square, to_square, QUIET_MOVE));
            }
            BPtargets &= (BPtargets - 1);
        }
        while (BPtargets2) {
            int to_square = __builtin_ctzll(BPtargets2);
            list.add_move(encode_move(to_square + 16, to_square, DOUBLE_PAWN_PUSH));
            BPtargets2 &= (BPtargets2 - 1);
        }

        uint64_t current_black_pawns = black_pawns;
        while (current_black_pawns) {
            int from_square = __builtin_ctzll(current_black_pawns);
            uint64_t valid_captures = board.pawn_attacks_black[from_square] & white_pieces;
            while (valid_captures) {
                int to_square = __builtin_ctzll(valid_captures);
                if (from_square / 8 == 1) {
                    promotions(board, list, BLACK, from_square, to_square, true);
                } else {
                    list.add_move(encode_move(from_square, to_square, CAPTURE_MOVE));
                }
                valid_captures &= (valid_captures - 1);
            }
            current_black_pawns &= (current_black_pawns - 1);
        }

        // --- BLACK KNIGHTS ---
        uint64_t black_knights = board.get_black_knights();
        while (black_knights) {
            int from_square = __builtin_ctzll(black_knights);
            uint64_t valid_moves = board.knight_attacks[from_square] & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_knights &= (black_knights - 1);
        }

        // --- BLACK KINGS ---
        uint64_t black_kings = board.get_black_kings();
        while (black_kings) {
            int from_square = __builtin_ctzll(black_kings);
            uint64_t valid_moves = board.king_attacks[from_square] & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_kings &= (black_kings - 1);
        }

        // --- BLACK ROOKS ---
        uint64_t black_rooks = board.get_black_rooks();
        while (black_rooks) {
            int from_square = __builtin_ctzll(black_rooks);
            board.init_rooks(all_occupied, from_square);
            uint64_t valid_moves = board.rook_attacks[from_square] & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_rooks &= (black_rooks - 1);
        }

        // --- BLACK BISHOPS ---
        uint64_t black_bishops = board.get_black_bishops();
        while (black_bishops) {
            int from_square = __builtin_ctzll(black_bishops);
            board.init_bishops(all_occupied, from_square);
            uint64_t valid_moves = board.bishop_attacks[from_square] & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_bishops &= (black_bishops - 1);
        }

        // --- BLACK QUEENS ---
        uint64_t black_queens = board.get_black_queens();
        while (black_queens) {
            int from_square = __builtin_ctzll(black_queens);
            board.init_queens(all_occupied, from_square);
            uint64_t valid_moves = board.queen_attacks[from_square] & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_queens &= (black_queens - 1);
        }
    }
}
