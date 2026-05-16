#include "bitboard.h"
#include "move.h"
#include "movesgen.h"
#include "uci.h"
#include <iostream>
#include <string>

static const char file_chars[] = "abcdefgh";

std::string square_to_string(int square) {
  int file = square % 8;
  int rank = square / 8;
  return std::string(1, file_chars[file]) + std::to_string(rank + 1);
}

std::string move_to_string(uint16_t move) {
  int from = get_move_from(move);
  int to = get_move_to(move);
  int flags = get_move_flags(move);
  std::string result = square_to_string(from) + square_to_string(to);
  switch (flags) {
  case PROMOTION_KNIGHT:
    result += 'n';
    break;
  case PROMOTION_BISHOP:
    result += 'b';
    break;
  case PROMOTION_ROOK:
    result += 'r';
    break;
  case PROMOTION_QUEEN:
    result += 'q';
    break;
  case PROMOTION_CAPTURE_KNIGHT:
    result += 'n';
    break;
  case PROMOTION_CAPTURE_BISHOP:
    result += 'b';
    break;
  case PROMOTION_CAPTURE_ROOK:
    result += 'r';
    break;
  case PROMOTION_CAPTURE_QUEEN:
    result += 'q';
    break;
  default:
    break;
  }
  return result;
}

int main() {
  init_zobrist();
  Board board;

  board.init_kings();
  board.init_knights();
  board.init_pawn_attacks();

  board.FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  int depth = 5;
  std::cout << "Starting Perft Divide to Depth " << depth << "...\n";

  MoveList root_moves;
  generate_moves(board, root_moves);

  uint64_t total_nodes = 0;
  for (int i = 0; i < root_moves.count; i++) {
    uint16_t move = root_moves.moves[i];
    if (board.make_move(move)) {
      uint64_t subtree = perft(board, depth - 1);
      board.unmake_move(move);
      std::cout << move_to_string(move) << ": " << subtree << "\n";
      total_nodes += subtree;
    }
  }

  std::cout << "\nTotal: " << total_nodes << "\n";
  std::cout << "Expected: 4865609\n";

  return 0;
}
