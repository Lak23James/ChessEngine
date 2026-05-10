#include "move.h" // We need this so it knows what our 16-bit moves are
#include "bitboard.h"//All  the static stuff is stored in here so it will be required when we need the static stuff

// The container that holds all generated moves
struct MoveList {
    uint16_t moves[256]; // Array of 16-bit moves
    int count = 0;       // How many moves are currently in the array
    // function to add new moves in the list of moves 
    void add_move(uint16_t move) {
        moves[count++] = move;
    }
};
void generate_moves(Board& board, MoveList& list){
    //calling for the information on the board
    uint64_t white_pieces = board.get_white_pieces();
    uint64_t black_pieces = board.get_black_pieces();
    uint64_t black_pawns=board.get_black_pawns();
    uint64_t white_pawns = board.get_white_pawns();
    uint64_t empty_squares = board.get_empty();
    uint64_t white_single_push_targets = (white_pawns << 8) & empty_squares;
    uint64_t WPtargets = white_single_push_targets; // The bitboard we generated earlier
    uint64_t black_single_push_targets = (black_pawns >>8) & empty_squares;
    uint64_t BPtargets=black_single_push_targets;
    uint64_t white_double_push_targets = (WPtargets << 8) & empty_squares & RANK_4;
    uint64_t black_double_push_targets = (BPtargets >> 8) & empty_squares & RANK_5;
    uint64_t WPtargets2 = white_double_push_targets;
    uint64_t BPtargets2 = black_double_push_targets; 
     //Modern CPUs have a specific instruction built into the silicon that can count how many zeros are sitting in front of the first 1 bit.
    //CTZ stands for Count Trailing Zeros.
    //In GCC and Clang, the function is __builtin_ctzll(bitboard).
     while (WPtargets) {
    // 1. Find the index of the first '1'
    int to_square = __builtin_ctzll(WPtargets);
    
    // 2. Logic: If it landed at 'to', it came from 'to - 8'
    int from_square = to_square - 8;
    
    // 3. Storing the move in   MoveList
    list.add_move(encode_move(from_square, to_square, QUIET_MOVE));
    
    // 4. "Pop" the bit we just processed
    WPtargets &= (WPtargets - 1);
}
   while(BPtargets){
    // 1. Find the index of the first '1'
    int to_square=__builtin_ctzll(BPtargets);
    // 2. Logic: If it landed at 'to', it came from 'to +8'
    int from_square = to_square + 8;
     // 3. Storing the move in the MoveList
    list.add_move(encode_move(from_square, to_square, QUIET_MOVE));
    
    // 4. "Pop" the bit we just processed
    BPtargets &= (BPtargets - 1);
   }
while (WPtargets2) {
        // 1. Find the index of the first '1'
        int to_square = __builtin_ctzll(WPtargets2);
        
        // 2. Logic: If it landed at 'to', it came from 'to - 16'
        int from_square = to_square - 16;
        
        // 3. Storing the move in MoveList (USING THE DOUBLE PUSH FLAG)
        list.add_move(encode_move(from_square, to_square, DOUBLE_PAWN_PUSH));
        
        // 4. "Pop" the bit we just processed
        WPtargets2 &= (WPtargets2 - 1);
    } // <-- THIS WAS THE MISSING BRACKET!

    while (BPtargets2) {
        // 1. Find the index of the first '1'
        int to_square = __builtin_ctzll(BPtargets2);
        
        // 2. Logic: If it landed at 'to', it came from 'to + 16'
        int from_square = to_square + 16;
        
        // 3. Storing the move in MoveList (USING THE DOUBLE PUSH FLAG)
        list.add_move(encode_move(from_square, to_square, DOUBLE_PAWN_PUSH));
        
        // 4. "Pop" the bit we just processed
        BPtargets2 &= (BPtargets2 - 1);
    }
   
    //       WHITE PAWN CAPTURES (Using LUT)
    uint64_t current_white_pawns = board.get_white_pawns();
    // black_pieces already declared at top of function

    while (current_white_pawns) {
        int from_square = __builtin_ctzll(current_white_pawns);
        
        // 1. Look up the static attacks
        // 2. Filter by where black pieces actually are
        uint64_t valid_captures = board.pawn_attacks_white[from_square] & black_pieces;

        while (valid_captures) {
            int to_square = __builtin_ctzll(valid_captures);
            list.add_move(encode_move(from_square, to_square, CAPTURE_MOVE));
            valid_captures &= (valid_captures - 1);
        }
        current_white_pawns &= (current_white_pawns - 1);
    }

  
    //       WHITE KNIGHTS (Using LUT)
    
    uint64_t white_knights = board.get_white_knights();
    // white_pieces already declared at top of function

    while (white_knights) {
        int from_square = __builtin_ctzll(white_knights);
        
        // 1. Look up the static attacks
        // 2. Filter OUT squares where our OWN pieces are standing
        uint64_t valid_moves = board.knight_attacks[from_square] & ~white_pieces;

        while (valid_moves) {
            int to_square = __builtin_ctzll(valid_moves);
            
            // Check if the landing square has an enemy piece to set the correct flag
            int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
            
            list.add_move(encode_move(from_square, to_square, flag));
            valid_moves &= (valid_moves - 1);
        }
        white_knights &= (white_knights - 1);
    }

   
    //       WHITE KINGS (Using LUT)
    
    uint64_t white_kings = board.get_white_kings();

    while (white_kings) {
        int from_square = __builtin_ctzll(white_kings);
        
        // 1. Look up attacks
        // 2. Filter out our own pieces
        uint64_t valid_moves = board.king_attacks[from_square] & ~white_pieces;

        while (valid_moves) {
            int to_square = __builtin_ctzll(valid_moves);
            
            int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
            
            list.add_move(encode_move(from_square, to_square, flag));
            valid_moves &= (valid_moves - 1);
        }
        white_kings &= (white_kings - 1);
    }
    
    //       BLACK PAWN CAPTURES (Using LUT)
    
    uint64_t current_black_pawns = board.get_black_pawns();
    // white_pieces already declared at top of function

    while (current_black_pawns) {
        int from_square = __builtin_ctzll(current_black_pawns);
        
        // 1. Look up Black's attack map
        // 2. Filter for actual White pieces
        uint64_t valid_captures = board.pawn_attacks_black[from_square] & white_pieces;

        while (valid_captures) {
            int to_square = __builtin_ctzll(valid_captures);
            list.add_move(encode_move(from_square, to_square, CAPTURE_MOVE));
            valid_captures &= (valid_captures - 1);
        }
        current_black_pawns &= (current_black_pawns - 1);
    }

    
    //       BLACK KNIGHTS
  
    uint64_t black_knights = board.get_black_knights();
    // black_pieces already declared at top of function

    while (black_knights) {
        int from_square = __builtin_ctzll(black_knights);
        
        // Filter OUT squares where our OWN black pieces are standing
        uint64_t valid_moves = board.knight_attacks[from_square] & ~black_pieces;

        while (valid_moves) {
            int to_square = __builtin_ctzll(valid_moves);
            int flag = (white_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
            list.add_move(encode_move(from_square, to_square, flag));
            valid_moves &= (valid_moves - 1);
        }
        black_knights &= (black_knights - 1);
    }

   
    //       BLACK KINGS
   
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

    //       SLIDING PIECES PREP

    uint64_t all_occupied = board.get_occupied();
    
   
    //       WHITE SLIDING PIECES
    
    
    // --- White Rooks ---
    uint64_t white_rooks = board.get_white_rooks();
    while (white_rooks) {
        int from_square = __builtin_ctzll(white_rooks);
        board.init_rooks(all_occupied, from_square); // Calculate dynamic rays
        uint64_t valid_moves = board.rook_attacks[from_square] & ~white_pieces;
        
        while (valid_moves) {
            int to_square = __builtin_ctzll(valid_moves);
            int flag = (black_pieces & (1ULL << to_square)) ? CAPTURE_MOVE : QUIET_MOVE;
            list.add_move(encode_move(from_square, to_square, flag));
            valid_moves &= (valid_moves - 1);
        }
        white_rooks &= (white_rooks - 1);
    }

    // --- White Bishops ---
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

    // --- White Queens ---
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

   
    //       BLACK SLIDING PIECES
 

    // --- Black Rooks ---
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

    // --- Black Bishops ---
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

    // --- Black Queens ---
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