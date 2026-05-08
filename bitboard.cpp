#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>

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
       // 1. Up 2, Right 1 (Shift Left 17)
        if (bitboard & ~FILE_H) attacks |= (bitboard << 17);
        // 2. Up 1, Right 2 (Shift Left 10)
        if (bitboard & ~FILE_GH) attacks |= (bitboard << 10);
        // 3. Up 2, Left 1 (Shift Left 15)
        if (bitboard & ~FILE_A) attacks |= (bitboard << 15);
        // 4. Up 1, Left 2 (Shift Left 6) 
        if (bitboard & ~FILE_AB) attacks |= (bitboard << 6);
        // 5. Down 2, Right 1 (Moving RIGHT, needs H wall)
        if (bitboard & ~FILE_H) attacks |= (bitboard >> 15);
        // 6. Down 1, Right 2 (Moving RIGHT, needs GH wall)
        if (bitboard & ~FILE_GH) attacks |= (bitboard >> 6);
        // 7. Down 2, Left 1 (Moving LEFT, needs A wall)
        if (bitboard & ~FILE_A) attacks |= (bitboard >> 17);
        // 8. Down 1, Left 2 (Moving LEFT, needs AB wall)
        if (bitboard & ~FILE_AB) attacks |= (bitboard >> 10);


        knight_attacks[square] = attacks;
      }
       


  }
    






};
    