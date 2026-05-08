#include "bitboard.cpp"
#include <iostream>
int main(){
    board_representation board;
    board.bitboard[WK] |= (1ULL << E1);
    board.bitboard[BK] |= (1ULL << E8);
    board.print_board();
    return 0;
}










