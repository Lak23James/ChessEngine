#include "movesgen.h"

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
    uint64_t white_pawns = board.get_white_pawns();
    uint64_t black_pawns = board.get_black_pawns();
    if (board.side_to_move == WHITE) {
        // --- WHITE PAWNS ---
        
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
            uint64_t valid_moves = board.get_rook_attacks(all_occupied, from_square) & ~white_pieces;
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
            uint64_t valid_moves = board.get_bishop_attacks(all_occupied, from_square) & ~white_pieces;
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
            uint64_t rook_atks = board.get_rook_attacks(all_occupied, from_square);
            uint64_t bishop_atks = board.get_bishop_attacks(all_occupied, from_square);
            uint64_t valid_moves = (rook_atks | bishop_atks) & ~white_pieces;
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
            uint64_t valid_moves = board.get_rook_attacks(all_occupied, from_square) & ~black_pieces;
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
            uint64_t valid_moves = board.get_bishop_attacks(all_occupied, from_square) & ~black_pieces;
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
            uint64_t rook_atks = board.get_rook_attacks(all_occupied, from_square);
            uint64_t bishop_atks = board.get_bishop_attacks(all_occupied, from_square);
            uint64_t valid_moves = (rook_atks | bishop_atks) & ~black_pieces;
            while (valid_moves) {
                int to_square = __builtin_ctzll(valid_moves);
                int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
                list.add_move(encode_move(from_square, to_square, flag));
                valid_moves &= (valid_moves - 1);
            }
            black_queens &= (black_queens - 1);
        }
    }
    
    if (board.enpassentsq != NO_SQUARE) {
        uint64_t ep_target = 1ULL << board.enpassentsq;

        if (board.side_to_move == WHITE) {
            uint64_t ep_left = ((white_pawns & ~Board::FILE_A) << 7) & ep_target;
            if (ep_left) {
                int to_sq = __builtin_ctzll(ep_left);
                list.add_move(encode_move(to_sq - 7, to_sq, EN_PASSANT));
            }

            uint64_t ep_right = ((white_pawns & ~Board::FILE_H) << 9) & ep_target;
            if (ep_right) {
                int to_sq = __builtin_ctzll(ep_right);
                list.add_move(encode_move(to_sq - 9, to_sq, EN_PASSANT));
            }
        } else {
            uint64_t ep_left = ((black_pawns & ~Board::FILE_H) >> 7) & ep_target;
            if (ep_left) {
                int to_sq = __builtin_ctzll(ep_left);
                list.add_move(encode_move(to_sq + 7, to_sq, EN_PASSANT));
            }

            uint64_t ep_right = ((black_pawns & ~Board::FILE_A) >> 9) & ep_target;
            if (ep_right) {
                int to_sq = __builtin_ctzll(ep_right);
                list.add_move(encode_move(to_sq + 9, to_sq, EN_PASSANT));
            }
        }
    }

    // Castling moves
    if (board.side_to_move == WHITE) {
        // White king side castling
        if (board.can_white_castle_king_side) {
            // Check if squares between king and rook are empty
            if (!(board.get_occupied() & ((1ULL << 5) | (1ULL << 6)))) {
                // Check if king is not in check and transit squares are not attacked
                if (!board.is_square_attacked(4, BLACK) &&  // e1
                    !board.is_square_attacked(5, BLACK) &&  // f1
                    !board.is_square_attacked(6, BLACK)) {  // g1
                    list.add_move(encode_move(4, 6, KING_CASTLE));
                }
            }
        }

        // White queen side castling
        if (board.can_white_castle_queen_side) {
            // Check if squares between king and rook are empty
            if (!(board.get_occupied() & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)))) {
                // Check if king is not in check and transit squares are not attacked
                if (!board.is_square_attacked(4, BLACK) &&  // e1
                    !board.is_square_attacked(3, BLACK) &&  // d1
                    !board.is_square_attacked(2, BLACK)) {  // c1
                    list.add_move(encode_move(4, 2, QUEEN_CASTLE));
                }
            }
        }
    } else {
        // Black king side castling
        if (board.can_black_castle_king_side) {
            // Check if squares between king and rook are empty
            if (!(board.get_occupied() & ((1ULL << 61) | (1ULL << 62)))) {
                // Check if king is not in check and transit squares are not attacked
                if (!board.is_square_attacked(60, WHITE) &&  // e8
                    !board.is_square_attacked(61, WHITE) &&  // f8
                    !board.is_square_attacked(62, WHITE)) {  // g8
                    list.add_move(encode_move(60, 62, KING_CASTLE));
                }
            }
        }

        // Black queen side castling
        if (board.can_black_castle_queen_side) {
            // Check if squares between king and rook are empty
            if (!(board.get_occupied() & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)))) {
                // Check if king is not in check and transit squares are not attacked
                if (!board.is_square_attacked(60, WHITE) &&  // e8
                    !board.is_square_attacked(59, WHITE) &&  // d8
                    !board.is_square_attacked(58, WHITE)) {  // c8
                    list.add_move(encode_move(60, 58, QUEEN_CASTLE));
                }
            }
        }
    }
}

// Check if a square is attacked by the given side
bool Board::is_square_attacked(int square, int side) const {
    uint64_t target = 1ULL << square;
    uint64_t all_occupied = occupied[BOTH];

    if (side == WHITE) {
        // Check white pawn attacks on this square
        // Use BLACK table: white pawns are BELOW and attack UPWARD into this square
        uint64_t white_pawns = bitboard[WP];
        if (pawn_attacks_black[square] & white_pawns) return true;

        // Check white knight attacks
        uint64_t white_knights = bitboard[WN];
        if (knight_attacks[square] & white_knights) return true;

        // Check white king attacks
        uint64_t white_king = bitboard[WK];
        if (king_attacks[square] & white_king) return true;

        // Check white rook and queen attacks (sliding pieces)
        uint64_t white_rooks = bitboard[WR];
        uint64_t white_queens = bitboard[WQ];
        uint64_t rook_atks = get_rook_attacks(all_occupied, square);
        if (rook_atks & (white_rooks | white_queens)) return true;

        // Check white bishop and queen attacks (sliding pieces)
        uint64_t bishop_atks = get_bishop_attacks(all_occupied, square);
        if (bishop_atks & (bitboard[WB] | white_queens)) return true;

        return false;
    } else {
        // Check black pawn attacks on this square
        // Use WHITE table: black pawns are ABOVE and attack DOWNWARD into this square
        uint64_t black_pawns = bitboard[BP];
        if (pawn_attacks_white[square] & black_pawns) return true;

        // Check black knight attacks
        uint64_t black_knights = bitboard[BN];
        if (knight_attacks[square] & black_knights) return true;

        // Check black king attacks
        uint64_t black_king = bitboard[BK];
        if (king_attacks[square] & black_king) return true;

        // Check black rook and queen attacks (sliding pieces)
        uint64_t black_rooks = bitboard[BR];
        uint64_t black_queens = bitboard[BQ];
        uint64_t rook_atks = get_rook_attacks(all_occupied, square);
        if (rook_atks & (black_rooks | black_queens)) return true;

        // Check black bishop and queen attacks (sliding pieces)
        uint64_t bishop_atks = get_bishop_attacks(all_occupied, square);
        if (bishop_atks & (bitboard[BB] | black_queens)) return true;

        return false;
    }
}
bool Board::make_move(uint16_t move) {
    int from_sq = get_move_from(move);
    int to_sq = get_move_to(move);
    int flag = get_move_flags(move);

    int moving_piece = piece_on_square[from_sq]; 
    if (moving_piece == -1) return false;

    int original_side = side_to_move;
    int original_enpassentsq = enpassentsq;
    bool original_castling[4] = {
        can_white_castle_king_side, can_white_castle_queen_side,
        can_black_castle_king_side, can_black_castle_queen_side
    };
    int original_castling_rights = (can_white_castle_king_side ? 1 : 0) | 
                                   (can_white_castle_queen_side ? 2 : 0) | 
                                   (can_black_castle_king_side ? 4 : 0) | 
                                   (can_black_castle_queen_side ? 8 : 0);
    uint64_t original_hash = hash_key;

    history[state_ply].enpassentsq = enpassentsq;
    history[state_ply].castling[0] = can_white_castle_king_side;
    history[state_ply].castling[1] = can_white_castle_queen_side;
    history[state_ply].castling[2] = can_black_castle_king_side;
    history[state_ply].castling[3] = can_black_castle_queen_side;
    history[state_ply].hash_key = hash_key;
    history[state_ply].halfmove_clock = halfmove_clock;
    history[state_ply].captured_piece = -1;

    if (enpassentsq != NO_SQUARE) hash_key ^= enpassant_keys[enpassentsq];
    enpassentsq = NO_SQUARE;

    bool was_capture = false;
    int captured_piece = -1;
    int capture_square = -1;
    if (flag == CAPTURE_MOVE || flag == EN_PASSANT ||
        (flag >= PROMOTION_CAPTURE_KNIGHT && flag <= PROMOTION_CAPTURE_QUEEN)) {
        was_capture = true;
        capture_square = (flag == EN_PASSANT)
            ? ((side_to_move == WHITE) ? to_sq - 8 : to_sq + 8)
            : to_sq;
        captured_piece = piece_on_square[capture_square];
        history[state_ply].captured_piece = captured_piece;
        if (captured_piece != -1) {
            bitboard[captured_piece] &= ~(1ULL << capture_square);
            occupied[captured_piece / 6] &= ~(1ULL << capture_square);
            occupied[BOTH] &= ~(1ULL << capture_square);
            piece_on_square[capture_square] = -1;
            hash_key ^= piece_keys[captured_piece][capture_square];
        }
    }

    uint64_t move_mask = (1ULL << from_sq) | (1ULL << to_sq);
    bitboard[moving_piece] ^= move_mask;
    occupied[moving_piece / 6] ^= move_mask;
    occupied[BOTH] ^= move_mask;
    piece_on_square[from_sq] = -1;
    piece_on_square[to_sq] = moving_piece;
    
    hash_key ^= piece_keys[moving_piece][from_sq];
    hash_key ^= piece_keys[moving_piece][to_sq];

    bool was_castle = false;
    int rook_from = -1;
    int rook_to = -1;
    int promoted_piece = -1;
    bool was_promotion = false;

    if (flag == DOUBLE_PAWN_PUSH) {
        enpassentsq = (side_to_move == WHITE) ? to_sq - 8 : to_sq + 8;
        hash_key ^= enpassant_keys[enpassentsq];
    } else if (flag == KING_CASTLE) {
        was_castle = true;
        if (side_to_move == WHITE) {
            rook_from = 7;
            rook_to = 5;
            uint64_t rook_mask = (1ULL << rook_from) | (1ULL << rook_to);
            bitboard[WR] ^= rook_mask;
            occupied[WHITE] ^= rook_mask;
            occupied[BOTH] ^= rook_mask;
            piece_on_square[rook_from] = -1;
            piece_on_square[rook_to] = WR;
            hash_key ^= piece_keys[WR][rook_from];
            hash_key ^= piece_keys[WR][rook_to];
        } else {
            rook_from = 63;
            rook_to = 61;
            uint64_t rook_mask = (1ULL << rook_from) | (1ULL << rook_to);
            bitboard[BR] ^= rook_mask;
            occupied[BLACK] ^= rook_mask;
            occupied[BOTH] ^= rook_mask;
            piece_on_square[rook_from] = -1;
            piece_on_square[rook_to] = BR;
            hash_key ^= piece_keys[BR][rook_from];
            hash_key ^= piece_keys[BR][rook_to];
        }
    } else if (flag == QUEEN_CASTLE) {
        was_castle = true;
        if (side_to_move == WHITE) {
            rook_from = 0;
            rook_to = 3;
            uint64_t rook_mask = (1ULL << rook_from) | (1ULL << rook_to);
            bitboard[WR] ^= rook_mask;
            occupied[WHITE] ^= rook_mask;
            occupied[BOTH] ^= rook_mask;
            piece_on_square[rook_from] = -1;
            piece_on_square[rook_to] = WR;
            hash_key ^= piece_keys[WR][rook_from];
            hash_key ^= piece_keys[WR][rook_to];
        } else {
            rook_from = 56;
            rook_to = 59;
            uint64_t rook_mask = (1ULL << rook_from) | (1ULL << rook_to);
            bitboard[BR] ^= rook_mask;
            occupied[BLACK] ^= rook_mask;
            occupied[BOTH] ^= rook_mask;
            piece_on_square[rook_from] = -1;
            piece_on_square[rook_to] = BR;
            hash_key ^= piece_keys[BR][rook_from];
            hash_key ^= piece_keys[BR][rook_to];
        }
    } else if (flag >= PROMOTION_KNIGHT && flag <= PROMOTION_CAPTURE_QUEEN) {
        was_promotion = true;
        bitboard[moving_piece] &= ~(1ULL << to_sq);
        hash_key ^= piece_keys[moving_piece][to_sq];
        if (flag == PROMOTION_KNIGHT || flag == PROMOTION_CAPTURE_KNIGHT) promoted_piece = (side_to_move == WHITE) ? WN : BN;
        else if (flag == PROMOTION_BISHOP || flag == PROMOTION_CAPTURE_BISHOP) promoted_piece = (side_to_move == WHITE) ? WB : BB;
        else if (flag == PROMOTION_ROOK || flag == PROMOTION_CAPTURE_ROOK) promoted_piece = (side_to_move == WHITE) ? WR : BR;
        else promoted_piece = (side_to_move == WHITE) ? WQ : BQ;
        bitboard[promoted_piece] |= (1ULL << to_sq);
        piece_on_square[to_sq] = promoted_piece;
        hash_key ^= piece_keys[promoted_piece][to_sq];
    }

    hash_key ^= castle_keys[original_castling_rights];

    if (moving_piece == WK) {
        can_white_castle_king_side = false;
        can_white_castle_queen_side = false;
    } else if (moving_piece == BK) {
        can_black_castle_king_side = false;
        can_black_castle_queen_side = false;
    } else if (moving_piece == WR) {
        if (from_sq == 0) can_white_castle_queen_side = false;
        if (from_sq == 7) can_white_castle_king_side = false;
    } else if (moving_piece == BR) {
        if (from_sq == 56) can_black_castle_queen_side = false;
        if (from_sq == 63) can_black_castle_king_side = false;
    }

    int new_castling_rights = (can_white_castle_king_side ? 1 : 0) | 
                              (can_white_castle_queen_side ? 2 : 0) | 
                              (can_black_castle_king_side ? 4 : 0) | 
                              (can_black_castle_queen_side ? 8 : 0);
    hash_key ^= castle_keys[new_castling_rights];

    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;
    hash_key ^= side_key;

    int king_square = (side_to_move == BLACK) ? __builtin_ctzll(bitboard[WK]) : __builtin_ctzll(bitboard[BK]);
    int enemy_side = side_to_move; // side_to_move is now the enemy side (it was toggled above)

    if (is_square_attacked(king_square, enemy_side)) {
        side_to_move = original_side;
        enpassentsq = original_enpassentsq;
        can_white_castle_king_side = original_castling[0];
        can_white_castle_queen_side = original_castling[1];
        can_black_castle_king_side = original_castling[2];
        can_black_castle_queen_side = original_castling[3];
        hash_key = original_hash;

        if (was_promotion) {
            bitboard[promoted_piece] &= ~(1ULL << to_sq);
            bitboard[moving_piece] |= (1ULL << from_sq);
            occupied[moving_piece / 6] |= (1ULL << from_sq);
            occupied[moving_piece / 6] &= ~(1ULL << to_sq);
            occupied[BOTH] &= ~(1ULL << to_sq);
            occupied[BOTH] |= (1ULL << from_sq);
            piece_on_square[from_sq] = moving_piece;
            if (was_capture && captured_piece != -1) {
                bitboard[captured_piece] |= (1ULL << capture_square);
                occupied[captured_piece / 6] |= (1ULL << capture_square);
                occupied[BOTH] |= (1ULL << capture_square);
                piece_on_square[capture_square] = captured_piece;
            } else {
                piece_on_square[to_sq] = -1;
            }
        } else {
            uint64_t unmove_mask = (1ULL << from_sq) | (1ULL << to_sq);
            bitboard[moving_piece] ^= unmove_mask;
            occupied[moving_piece / 6] ^= unmove_mask;
            occupied[BOTH] ^= unmove_mask;
            piece_on_square[from_sq] = moving_piece;
            piece_on_square[to_sq] = -1;
            if (was_capture && captured_piece != -1) {
                bitboard[captured_piece] |= (1ULL << capture_square);
                occupied[captured_piece / 6] |= (1ULL << capture_square);
                occupied[BOTH] |= (1ULL << capture_square);
                piece_on_square[capture_square] = captured_piece;
            }
        }

        if (was_castle) {
            uint64_t unrook_mask = (1ULL << rook_from) | (1ULL << rook_to);
            bitboard[(original_side == WHITE) ? WR : BR] ^= unrook_mask;
            occupied[(original_side == WHITE) ? WHITE : BLACK] ^= unrook_mask;
            occupied[BOTH] ^= unrook_mask;
            piece_on_square[rook_to] = -1;
            piece_on_square[rook_from] = (original_side == WHITE) ? WR : BR;
        }

        return false;
    }

    state_ply++;
    return true;
}

void Board::unmake_move(uint16_t move) {
    state_ply--;
    
    int from_sq = get_move_from(move);
    int to_sq = get_move_to(move);
    int flag = get_move_flags(move);

    // Toggle side back
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;

    // Restore state variables instantly from history stack
    enpassentsq = history[state_ply].enpassentsq;
    can_white_castle_king_side = history[state_ply].castling[0];
    can_white_castle_queen_side = history[state_ply].castling[1];
    can_black_castle_king_side = history[state_ply].castling[2];
    can_black_castle_queen_side = history[state_ply].castling[3];
    hash_key = history[state_ply].hash_key;
    halfmove_clock = history[state_ply].halfmove_clock;
    int captured_piece = history[state_ply].captured_piece;

    int moving_piece = piece_on_square[to_sq];

    // If it was a promotion, the moving piece we are putting back is a Pawn
    if (flag >= PROMOTION_KNIGHT && flag <= PROMOTION_CAPTURE_QUEEN) {
        bitboard[moving_piece] &= ~(1ULL << to_sq); // Remove promoted piece from to_sq
        moving_piece = (side_to_move == WHITE) ? WP : BP;
        bitboard[moving_piece] |= (1ULL << to_sq); // Place pawn on to_sq so XOR works correctly
        piece_on_square[to_sq] = moving_piece;
    }

    // Move the piece backward using XOR mask
    uint64_t unmove_mask = (1ULL << from_sq) | (1ULL << to_sq);
    bitboard[moving_piece] ^= unmove_mask;
    occupied[moving_piece / 6] ^= unmove_mask;
    occupied[BOTH] ^= unmove_mask;
    piece_on_square[from_sq] = moving_piece;
    piece_on_square[to_sq] = -1;

    // Put captured piece back
    if (captured_piece != -1) {
        int capture_square = (flag == EN_PASSANT)
            ? ((side_to_move == WHITE) ? to_sq - 8 : to_sq + 8)
            : to_sq;
        bitboard[captured_piece] |= (1ULL << capture_square);
        occupied[captured_piece / 6] |= (1ULL << capture_square);
        occupied[BOTH] |= (1ULL << capture_square);
        piece_on_square[capture_square] = captured_piece;
    }

    // Uncastle the rook
    if (flag == KING_CASTLE) {
        int rook_from, rook_to;
        if (side_to_move == WHITE) { rook_from = 7; rook_to = 5; }
        else { rook_from = 63; rook_to = 61; }
        uint64_t unrook_mask = (1ULL << rook_from) | (1ULL << rook_to);
        bitboard[(side_to_move == WHITE) ? WR : BR] ^= unrook_mask;
        occupied[side_to_move] ^= unrook_mask;
        occupied[BOTH] ^= unrook_mask;
        piece_on_square[rook_to] = -1;
        piece_on_square[rook_from] = (side_to_move == WHITE) ? WR : BR;
    } else if (flag == QUEEN_CASTLE) {
        int rook_from, rook_to;
        if (side_to_move == WHITE) { rook_from = 0; rook_to = 3; }
        else { rook_from = 56; rook_to = 59; }
        uint64_t unrook_mask = (1ULL << rook_from) | (1ULL << rook_to);
        bitboard[(side_to_move == WHITE) ? WR : BR] ^= unrook_mask;
        occupied[side_to_move] ^= unrook_mask;
        occupied[BOTH] ^= unrook_mask;
        piece_on_square[rook_to] = -1;
        piece_on_square[rook_from] = (side_to_move == WHITE) ? WR : BR;
    }
}

/*uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1ULL;
    
    MoveList list;
    generate_moves(board, list);
    
    uint64_t nodes = 0;
    for (int i = 0; i < list.count; i++) {
        if (board.make_move(list.moves[i])) {
            nodes += perft(board, depth - 1);
            board.unmake_move(list.moves[i]);
        }
    }
    return nodes;
}

// void perft_divide(Board& board, int depth) {
//     std::cout << "\n--- Perft Divide Depth " << depth << " ---\n";
//     MoveList list;
//     generate_moves(board, list);
//     
//     uint64_t total_nodes = 0;
//     for (int i = 0; i < list.count; i++) {
//         if (board.make_move(list.moves[i])) {
//             uint64_t nodes = perft(board, depth - 1);
//             int from = get_move_from(list.moves[i]);
//             int to = get_move_to(list.moves[i]);
//             
//             char file_from = 'a' + (from % 8);
//             char rank_from = '1' + (from / 8);
//             char file_to = 'a' + (to % 8);
//             char rank_to = '1' + (to / 8);
//             
//             std::cout << file_from << rank_from << file_to << rank_to << ": " << nodes << "\n";
//             total_nodes += nodes;
//             
//             board.unmake_move(list.moves[i]);
//         }
//     }
//     std::cout << "\nTotal Nodes: " << total_nodes << "\n";
// }*/