#include "bitboard.h"
#include <iostream>
int main(){
    Board board;
    board.add_to_bitboard(WK, (1ULL << E1));
    board.add_to_bitboard(BK, (1ULL << E8));
    board.init_kings();
    board.init_knights();
    std::cout << "King Attacks from D4:\n";
    board.print_attacks(board.king_attacks[27]);
    std::cout << "Knight Attacks from D4:\n";
    board.print_attacks(board.knight_attacks[27]);
    board.print_board();
    return 0;
};










