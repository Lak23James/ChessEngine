#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include "type.h"

// WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5, BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
// 0 to 63 for squares A1 to H8 0-=> A1, 1=>B1, ..., 63=>H8
const char ascii_pieces[13] = "PNBRQKpnbrqk";
class board_representation{
public:
    uint64_t FILE_A = 0x0101010101010101ULL;
    uint64_t FILE_H = 0x8080808080808080ULL;
    uint64_t FILE_GH = 0xC0C0C0C0C0C0C0C0ULL;
    uint64_t FILE_AB = 0x0303030303030303ULL;
    /// 0-5 for white pieces, 6-11 for black pieces{Representation of pieces}: 
    // WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5, BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
      uint64_t bitboard[12]={0};
      /// 0 for white pieces, 1 for black pieces,2 for double occupancy
      uint64_t occupied[3]={0};
      //
      uint64_t knight_attacks[64];
      uint64_t king_attacks[64];
      uint64_t rook_attacks[64];
      uint64_t bishop_attacks[64];
      uint64_t queen_attacks[64];
     //function to print the board
    void print_board() {
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
    void print_attacks(uint64_t mask) {
    std::cout << "\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << "  "; // Print the rank numbers on the left
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            
            // If the mask has a 1 at this exact square, print a 1. Otherwise, print a dot.
            if (mask & (1ULL << square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n"; // Move to the next line after finishing a rank
    }
    std::cout << "\n   a b c d e f g h\n\n"; // Print the file letters at the bottom
}
    // The FEN Parser
    void FEN(std::string fen) {
      // 1. Reset everything to zero before loading a new position
      for (int i = 0; i < 12; i++) bitboard[i] = 0ULL;
      for (int i = 0; i < 3; i++) occupied[i] = 0ULL;
     
      
      int rank=7;
      int file=0;
      // 2. Iterate through each character of the FEN string
      for(char c:fen){
        // Stop parsing the board if we hit a space
        if(c==' ')break;

        if(c=='/'){
         rank--;
         file=0;
        }
       else if(std::isdigit(static_cast<unsigned char>(c))){
         file += (c - '0');
      }
      else{
        // 3. Find which piece this character represents
        int piece_index=0;
        for(int j=0;j<12;j++){
          if(ascii_pieces[j]==c){
            piece_index=j;
            break;
          }
        }
        // Calculate the 0-63 index
        //4. Place the piece on the specific bitboard
        bitboard[piece_index]|=(1ULL<<(rank*8+file));
        // 5. Update occupancy bitboards
        occupied[piece_index/6]|=(1ULL<<(rank*8+file));
        occupied[BOTH] |= (1ULL << (rank*8+file));
        file++;// Move to the next square
       }

      }
      }
    //function to initialize knight attacks
    void init_knights(){
      for(int square=0;square<64;square++)
      {uint64_t bitboard = (1ULL << square);
       uint64_t attacks = 0;
       // 1. Up 2, Right 1 (Shift Left 17) (not on H file)
        if (bitboard & ~FILE_H) attacks |= (bitboard << 17);
        // 2. Up 1, Right 2 (Shift Left 10) (not on GH file)
        if (bitboard & ~FILE_GH) attacks |= (bitboard << 10);
        // 3. Up 2, Left 1 (Shift Left 15) (not on A file)
        if (bitboard & ~FILE_A) attacks |= (bitboard << 15);
        // 4. Up 1, Left 2 (Shift Left 6) (not on AB file)
        if (bitboard & ~FILE_AB) attacks |= (bitboard << 6);
        // 5. Down 2, Right 1 (Moving RIGHT, needs H wall) (Shift Right 15)
        if (bitboard & ~FILE_H) attacks |= (bitboard >> 15);
        // 6. Down 1, Right 2 (Moving RIGHT, needs GH wall)  
        if (bitboard & ~FILE_GH) attacks |= (bitboard >> 6);
        // 7. Down 2, Left 1 (Moving LEFT, needs A wall)
        if (bitboard & ~FILE_A) attacks |= (bitboard >> 17);
        // 8. Down 1, Left 2 (Moving LEFT, needs AB wall)
        if (bitboard & ~FILE_AB) attacks |= (bitboard >> 10);


        knight_attacks[square] = attacks;
      }}
    //function for moves of the king 
    void init_kings(){
      for(int square=0;square<64;square++) {
        uint64_t bitboard = (1ULL << square);
        uint64_t attacks = 0;

        attacks |= (bitboard << 8); // Up
        attacks |= (bitboard >> 8); // Down

        // if not on H file, can move right
        if (bitboard & ~FILE_H) {
          attacks |= (bitboard << 1);  // Right
          attacks |= (bitboard << 9);  // Up-Right
          attacks |= (bitboard >> 7);  // Down-Right
        }

        // if not on A file, can move left
        if (bitboard & ~FILE_A) {
          attacks |= (bitboard >> 1);  // Left
          attacks |= (bitboard << 7);  // Up-Left
          attacks |= (bitboard >> 9);  // Down-Left
        }

        king_attacks[square] = attacks;
      }
    }
    //function to initialize rook attacks
    void init_rooks(uint64_t block ,int rook_square ){
         //Ray tracing
         int rank=rook_square/8;
         int file=rook_square%8;
         for(int i=rank-1;i>=0;i--)
            { rook_attacks[rook_square]|=(1ULL<<(i*8+file));
              if((block & (1ULL << (i*8+file)))){
                break;
              }
         
          }
          for(int i=rank+1;i<8;i++)
              { rook_attacks[rook_square]|=(1ULL<<(i*8+file));
                if((block & (1ULL << (i*8+file)))){
                  break;
                }
          
            }
          for(int j=file-1;j>=0;j--)
              { rook_attacks[rook_square]|=(1ULL<<(rank*8+j));
                if((block & (1ULL << (rank*8+j)))){
                  break;
                }
          
            }
          for(int j=file+1;j<8;j++)
              { rook_attacks[rook_square]|=(1ULL<<(rank*8+j));
                if((block & (1ULL << (rank*8+j)))){
                  break;
                }
          
            }
          


    }
    //function to initialize bishop attacks
    void init_bishops(uint64_t block, int square) {
    int rank = square / 8;
    int file = square % 8;

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
     // function to initialize queen attacks
     //A queen's attacks are just the combination of rook and bishop attacks
      void init_queens(uint64_t block, int square) {
          init_rooks(block, square);
          init_bishops(block, square);
          queen_attacks[square] = rook_attacks[square] | bishop_attacks[square];
      }
};
 