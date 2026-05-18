#include "bitboard.h"
#include "evaluate.h"
#include "uci.h"
#include <cstdlib>
#include <ctime>

int main() {
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
