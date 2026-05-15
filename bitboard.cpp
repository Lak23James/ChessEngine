#include "bitboard.h"
#include <sstream>
// Function to print the board
void Board::print_board() {
    std::cout << "\n";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << "  ";

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece_on_this_square = piece_on_square[square];

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
    for (int i = 0; i < 64; i++) piece_on_square[i] = -1;
    
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
        }
         else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += (c - '0');
        }
         else {
            int piece_index = 0;
            for (int j = 0; j < 12; j++) {
                if (ascii_pieces[j] == c) {
                    piece_index = j;
                    break;
                }
            }
            int square = rank * 8 + file;
            bitboard[piece_index] |= (1ULL << square);
            occupied[piece_index / 6] |= (1ULL << square);
            occupied[BOTH] |= (1ULL << square);
            piece_on_square[square] = piece_index;
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
// 1. Change 'void' to 'uint64_t'
uint64_t Board::get_rook_attacks(uint64_t block, int rook_square) {
    
    // 2. Create a temporary local variable instead of touching the array
    uint64_t current_attacks = 0ULL; 
    int rank = rook_square / 8;
    int file = rook_square % 8;
    
    for (int i = rank - 1; i >= 0; i--) {
        current_attacks |= (1ULL << (i * 8 + file));
        if ((block & (1ULL << (i * 8 + file)))) break;
    }
    for (int i = rank + 1; i < 8; i++) {
        current_attacks |= (1ULL << (i * 8 + file));
        if ((block & (1ULL << (i * 8 + file)))) break;
    }
    for (int j = file - 1; j >= 0; j--) {
        current_attacks |= (1ULL << (rank * 8 + j));
        if ((block & (1ULL << (rank * 8 + j)))) break;
    }
    for (int j = file + 1; j < 8; j++) {
        current_attacks |= (1ULL << (rank * 8 + j));
        if ((block & (1ULL << (rank * 8 + j)))) break;
    }
    
    // 3. Return the calculated mask directly
    return current_attacks;
}

// Function to generate bishop attacks on the fly (Pure Function - Safe for Search)
uint64_t Board::get_bishop_attacks(uint64_t block, int square) {
    uint64_t current_attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    
    // Up-Right
    for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
        current_attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Down-Right
    for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
        current_attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Up-Left
    for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
        current_attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    // Down-Left
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        current_attacks |= (1ULL << (r * 8 + f));
        if (block & (1ULL << (r * 8 + f))) break;
    }
    
    return current_attacks;
}

// Function to initialize pawn attacks
void Board::init_pawn_attacks() {
    for (int square = 0; square < 64; square++) {
        uint64_t bb = (1ULL << square);
        uint64_t white_attacks = 0;
        uint64_t black_attacks = 0;
        
        int rank = square / 8;
        int file = square % 8;
        
        // White pawns attack diagonally upward
        if (rank < 7) {  // Not on rank 8
            if (file > 0) {  // Can attack to the left
                white_attacks |= (bb << 7);
            }
            if (file < 7) {  // Can attack to the right
                white_attacks |= (bb << 9);
            }
        }
        
        // Black pawns attack diagonally downward
        if (rank > 0) {  // Not on rank 1
            if (file > 0) {  // Can attack to the left
                black_attacks |= (bb >> 9);
            }
            if (file < 7) {  // Can attack to the right
                black_attacks |= (bb >> 7);
            }
        }
        
        pawn_attacks_white[square] = white_attacks;
        pawn_attacks_black[square] = black_attacks;
    }
}



