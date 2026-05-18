#include "bitboard.h"
#include "evaluate.h"
#include "uci.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
  // === CRITICAL FOR GUI COMPATIBILITY ===
  // Disable ALL buffering on stdout so Arena/Lichess pipes receive data immediately
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  std::ios_base::sync_with_stdio(true);       // Keep C and C++ streams synced
  std::cout.setf(std::ios::unitbuf);           // Flush after every << operation
  std::cin.tie(&std::cout);                    // Flush cout before every cin read

  init_zobrist();
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  Board board;
  board.init_kings();
  board.init_knights();
  board.init_pawn_attacks();
  board.FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  UCI::loop(board);
  return 0;
}
