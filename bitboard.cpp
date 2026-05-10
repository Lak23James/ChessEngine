#include "bitboard.h"
#include <sstream>

// Function to print the board
void Board::print_board() {
    std::cout << "\n";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << "  ";

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece_on_this_square = -1;

            for (int piece = WP; piece <= BK; piece++) {
                if (bitboard[piece] & (1ULL << square)) {
                    piece_on_this_square = piece;
                    break;
                }
            }

            if (piece_on_this_square == -1) {
                std::cout << ". ";
            } else {
                std::cout << ascii_pieces[piece_on_this_square] << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

void Board::print_attacks(uint64_t mask) {
    std::cout << "\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << "  ";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            // Print 1 for attacked squares, dot for empty
            if (mask & (1ULL << square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

// The FEN Parser
void Board::FEN(const std::string& fen) {
    // Reset all bitboards and state
    for (int i = 0; i < 12; i++) bitboard[i] = 0ULL;
    for (int i = 0; i < 3; i++) occupied[i] = 0ULL;
    
    // Default to false before parsing
    can_white_castle_king_side = false;
    can_white_castle_queen_side = false;
    can_black_castle_king_side = false;
    can_black_castle_queen_side = false;
    enpassentsq = NO_SQUARE;
    halfmove_clock = 0;

    std::istringstream ss(fen);
    std::string piece_placement, active_color, castling, en_passant, halfmove, fullmove;
    
    // This cleanly splits the FEN string by spaces into 6 parts
    ss >> piece_placement >> active_color >> castling >> en_passant >> halfmove >> fullmove;

    // 1. Parse Piece Placement
    int rank = 7;
    int file = 0;
    for (char c : piece_placement) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += (c - '0');
        } else {
            int piece_index = 0;
            for (int j = 0; j < 12; j++) {
                if (ascii_pieces[j] == c) {
                    piece_index = j;
                    break;
                }
            }
            bitboard[piece_index] |= (1ULL << (rank * 8 + file));
            occupied[piece_index / 6] |= (1ULL << (rank * 8 + file));
            occupied[BOTH] |= (1ULL << (rank * 8 + file));
            file++;
        }
    }

    // 2. Parse Side to Move
    if (active_color == "w") {
        side_to_move = WHITE;
    } else {
        side_to_move = BLACK;
    }

    // 3. Parse Castling Rights
    if (castling != "-") {
        for (char c : castling) {
            if (c == 'K') can_white_castle_king_side = true;
            else if (c == 'Q') can_white_castle_queen_side = true;
            else if (c == 'k') can_black_castle_king_side = true;
            else if (c == 'q') can_black_castle_queen_side = true;
        }
    }

    // 4. Parse En Passant
    if (en_passant != "-") {
        int ep_file = en_passant[0] - 'a';
        int ep_rank = en_passant[1] - '1';
        enpassentsq = ep_rank * 8 + ep_file;
    }

    // 5. Parse Halfmove clock (optional but good)
    if (!halfmove.empty()) {
        halfmove_clock = std::stoi(halfmove);
    }
}
// Precomputed lookup tables (LUT)
void Board::init_knights() {
    for (int square = 0; square < 64; square++) {
        uint64_t bb = (1ULL << square);
        uint64_t attacks = 0;
        // 1. Up 2, Right 1 (Shift Left 17) (not on H file)
        if (bb & ~FILE_H) attacks |= (bb << 17);
        // 2. Up 1, Right 2 (Shift Left 10) (not on GH file)
        if (bb & ~FILE_GH) attacks |= (bb << 10);
        // 3. Up 2, Left 1 (Shift Left 15) (not on A file)
        if (bb & ~FILE_A) attacks |= (bb << 15);
        // 4. Up 1, Left 2 (Shift Left 6) (not on AB file)
        if (bb & ~FILE_AB) attacks |= (bb << 6);
        // 5. Down 2, Right 1 (Moving RIGHT, needs H wall) (Shift Right 15)
        if (bb & ~FILE_H) attacks |= (bb >> 15);
        // 6. Down 1, Right 2 (Moving RIGHT, needs GH wall)
        if (bb & ~FILE_GH) attacks |= (bb >> 6);
        // 7. Down 2, Left 1 (Moving LEFT, needs A wall)
        if (bb & ~FILE_A) attacks |= (bb >> 17);
        // 8. Down 1, Left 2 (Moving LEFT, needs AB wall)
        if (bb & ~FILE_AB) attacks |= (bb >> 10);

        knight_attacks[square] = attacks;
    }
}

// Function for moves of the king
void Board::init_kings() {
    for (int square = 0; square < 64; square++) {
        uint64_t bb = (1ULL << square);
        uint64_t attacks = 0;

        attacks |= (bb << 8); // Up
        attacks |= (bb >> 8); // Down

        // if not on H file, can move right
        if (bb & ~FILE_H) {
            attacks |= (bb << 1);  // Right
            attacks |= (bb << 9);  // Up-Right
            attacks |= (bb >> 7);  // Down-Right
        }

        // if not on A file, can move left
        if (bb & ~FILE_A) {
            attacks |= (bb >> 1);  // Left
            attacks |= (bb << 7);  // Up-Left
            attacks |= (bb >> 9);  // Down-Left
        }

        king_attacks[square] = attacks;
    }
}

// Function to initialize rook attacks
void Board::init_rooks(uint64_t block, int rook_square) {
    // Ray tracing
    rook_attacks[rook_square] = 0ULL;
    int rank = rook_square / 8;
    int file = rook_square % 8;
    for (int i = rank - 1; i >= 0; i--) {
        rook_attacks[rook_square] |= (1ULL << (i * 8 + file));
        if ((block & (1ULL << (i * 8 + file)))) {
            break;
        }
    }
    for (int i = rank + 1; i < 8; i++) {
        rook_attacks[rook_square] |= (1ULL << (i * 8 + file));
        if ((block & (1ULL << (i * 8 + file)))) {
            break;
        }
    }
    for (int j = file - 1; j >= 0; j--) {
        rook_attacks[rook_square] |= (1ULL << (rank * 8 + j));
        if ((block & (1ULL << (rank * 8 + j)))) {
            break;
        }
    }
    for (int j = file + 1; j < 8; j++) {
        rook_attacks[rook_square] |= (1ULL << (rank * 8 + j));
        if ((block & (1ULL << (rank * 8 + j)))) {
            break;
        }
    }
}

// Function to initialize bishop attacks
void Board::init_bishops(uint64_t block, int square) {
    int rank = square / 8;
    int file = square % 8;
    bishop_attacks[square] = 0ULL;
    // Up-Right
    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        bishop_attacks[square] |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Down-Right
    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        bishop_attacks[square] |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Up-Left
    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        bishop_attacks[square] |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Down-Left
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        bishop_attacks[square] |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
}

// Function to initialize queen attacks
// A queen's attacks are just the combination of rook and bishop attacks
void Board::init_queens(uint64_t block, int square) {
    init_rooks(block, square);
    init_bishops(block, square);
    queen_attacks[square] = rook_attacks[square] | bishop_attacks[square];
}

// Function to initialize pawn moves
void Board::init_pawn_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        uint64_t b = (1ULL << sq);

        uint64_t white_attacks = 0;
        uint64_t black_attacks = 0;

        // --- WHITE PAWNS (Shift Left / Add to index) ---
        // Up-Left (+7). Mask: Cannot be on A-file.
        if (b & ~FILE_A) white_attacks |= (b << 7);

        // Up-Right (+9). Mask: Cannot be on H-file.
        if (b & ~FILE_H) white_attacks |= (b << 9);

        // --- BLACK PAWNS (Shift Right / Subtract from index) ---
        // Down-Left (-9). Mask: Cannot be on A-file.
        if (b & ~FILE_A) black_attacks |= (b >> 9);

        // Down-Right (-7). Mask: Cannot be on H-file.
        if (b & ~FILE_H) black_attacks |= (b >> 7);

        pawn_attacks_white[sq] = white_attacks;
        pawn_attacks_black[sq] = black_attacks;
    }
}