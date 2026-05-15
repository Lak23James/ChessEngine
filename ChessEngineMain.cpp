#include "bitboard.h"
#include "move.h"
#include <iostream>

// Forward declare perft function from movesgen.cpp
uint64_t perft(Board& board, int depth);

int main() {
    init_zobrist();
    Board board;

    // Call init functions!
    board.init_kings();
    board.init_knights();
    board.init_pawn_attacks();

    board.FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int depth = 5;
    std::cout << "Starting Perft to Depth " << depth << "...\n";
    uint64_t nodes = perft(board, depth);
    
    std::cout << "Nodes: " << nodes << "\n";
    std::cout << "Target for Depth 4: 197281\n";

    return 0;
}
