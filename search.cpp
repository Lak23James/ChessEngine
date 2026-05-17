#include "search.h"
#include "bitboard.h"
#include "evaluate.h"
#include "movesgen.h"
#include "tt.h"
#include <algorithm>

using Move = uint16_t;

// Definition of the transposition table declared in tt.h
TTEntry tt[TT_SIZE];

// Global variable to store the best move found at the root
Move best_move_found = 0;

// Probe TT
static bool probe_tt(uint64_t key, int depth, int alpha, int beta, int& score, int& best_move) {
    int index = key % TT_SIZE;
    TTEntry& entry = tt[index];

    if (entry.key == key) {
        best_move = entry.best_move;
        if (entry.depth >= depth) {
            if (entry.flag == TT_EXACT) {
                score = entry.score;
                return true;
            }
            if (entry.flag == TT_ALPHA && entry.score <= alpha) {
                score = alpha;
                return true;
            }
            if (entry.flag == TT_BETA && entry.score >= beta) {
                score = beta;
                return true;
            }
        }
    }
    return false;
}

// Store TT
static void store_tt(uint64_t key, int depth, int score, int flag, int best_move) {
    int index = key % TT_SIZE;
    TTEntry& entry = tt[index];

    // Depth-preferred replacement strategy
    if (entry.key != key || entry.depth <= depth) {
        entry.key = key;
        entry.score = score;
        entry.depth = depth;
        entry.flag = flag;
        entry.best_move = best_move;
    }
}

static inline bool is_king_in_check(const Board &board) {
  int king_square = -1;
  int enemy_side = (board.side_to_move == WHITE) ? BLACK : WHITE;

  if (board.side_to_move == WHITE) {
    uint64_t kings = board.get_white_kings();
    if (kings == 0)
      return false;
    king_square = __builtin_ctzll(kings);
  } else {
    uint64_t kings = board.get_black_kings();
    if (kings == 0)
      return false;
    king_square = __builtin_ctzll(kings);
  }

  return board.is_square_attacked(king_square, enemy_side);
}

static MoveList generate_legal_moves(Board &board) {
  MoveList result;
  MoveList all_moves;
  generate_moves(board, all_moves);

  for (int i = 0; i < all_moves.count; ++i) {
    Move move = all_moves.moves[i];
    if (board.make_move(move)) {
      board.unmake_move(move);
      result.add_move(move);
    }
  }
  return result;
}

// Approximate centipawn values for Delta Pruning and MVV-LVA
const int PIECE_VALUES[12] = {
    100, 320, 330,
    500, 900, 20000, // White: Pawn, Knight, Bishop, Rook, Queen, King
    100, 320, 330,
    500, 900, 20000 // Black: Pawn, Knight, Bishop, Rook, Queen, King
};

// MVV-LVA move ordering helper (prioritizes tt_move above all other captures)
static void sort_moves(const Board &board, MoveList &list, int tt_move = 0) {
  int scores[256] = {0};
  for (int i = 0; i < list.count; i++) {
    Move move = list.moves[i];

    if (move == tt_move) {
      scores[i] = 100000;
      continue;
    }

    int flags = get_move_flags(move);
    int from = get_move_from(move);
    int to = get_move_to(move);
    int victim = board.get_piece_on_square(to);
    int aggressor = board.get_piece_on_square(from);

    int score = 0;
    if (flags == CAPTURE_MOVE || (flags >= PROMOTION_CAPTURE_KNIGHT &&
                                  flags <= PROMOTION_CAPTURE_QUEEN)) {
      if (victim != -1 && aggressor != -1) {
        score = PIECE_VALUES[victim] * 10 - PIECE_VALUES[aggressor] + 1000;
      } else {
        score = 1000;
      }
    } else if (flags == EN_PASSANT) {
      score = 100 * 10 - 100 + 1000;
    } else if (flags >= PROMOTION_KNIGHT && flags <= PROMOTION_QUEEN) {
      score = 900;
    }
    scores[i] = score;
  }

  // Simple selection sort
  for (int i = 0; i < list.count - 1; i++) {
    for (int j = i + 1; j < list.count; j++) {
      if (scores[j] > scores[i]) {
        std::swap(scores[i], scores[j]);
        std::swap(list.moves[i], list.moves[j]);
      }
    }
  }
}

// 1. QUIESCENCE SEARCH (Internal Helper - Static Linkage)
static int quiescence(int alpha, int beta, Board &board) {
  bool in_check = is_king_in_check(board);
  int stand_pat = 0;

  // Standing Pat (Safety net)
  if (!in_check) {
    int color_multiplier = (board.side_to_move == WHITE) ? 1 : -1;
    stand_pat = PSQT::evaluate(board) * color_multiplier;
    if (stand_pat >= beta) {
      return beta;
    }
    if (stand_pat > alpha) {
      alpha = stand_pat;
    }
  }

  // Move Generation
  MoveList all_moves;
  generate_moves(board, all_moves);

  // Filter tactical moves (captures/promotions) if not in check
  MoveList list;
  for (int i = 0; i < all_moves.count; i++) {
    Move move = all_moves.moves[i];
    int flags = get_move_flags(move);
    bool is_capture = (flags == CAPTURE_MOVE || flags == EN_PASSANT ||
                       (flags >= PROMOTION_CAPTURE_KNIGHT &&
                        flags <= PROMOTION_CAPTURE_QUEEN));
    bool is_promotion =
        (flags >= PROMOTION_KNIGHT && flags <= PROMOTION_CAPTURE_QUEEN);

    if (in_check || is_capture || is_promotion) {
      list.add_move(move);
    }
  }

  // Move Ordering
  sort_moves(board, list);

  int legal_moves_played = 0;

  // Alpha-Beta Loop
  for (int i = 0; i < list.count; i++) {
    Move move = list.moves[i];
    int flags = get_move_flags(move);
    bool is_promotion =
        (flags >= PROMOTION_KNIGHT && flags <= PROMOTION_CAPTURE_QUEEN);

    // Delta Pruning (Only if not in check)
    if (!in_check && !is_promotion) {
      int captured_piece_value = 0;
      if (flags == EN_PASSANT) {
        captured_piece_value = 100; // En passant always captures a pawn
      } else {
        int to_sq = get_move_to(move);
        int captured_piece = board.get_piece_on_square(to_sq);
        if (captured_piece != -1) {
          captured_piece_value = PIECE_VALUES[captured_piece];
        }
      }

      // If standing pat + value of captured piece + 200 (safety margin)
      // still cannot beat alpha, this capture is completely futile.
      if (stand_pat + captured_piece_value + 200 <= alpha) {
        continue; // Skip evaluating this branch entirely
      }
    }

    // Execute the move
    if (!board.make_move(move)) {
      continue;
    }

    legal_moves_played++;

    // Recursive Negamax call
    int score = -quiescence(-beta, -alpha, board);

    board.unmake_move(move);

    // Alpha-Beta Cutoffs
    if (score >= beta) {
      return beta;
    }
    if (score > alpha) {
      alpha = score;
    }
  }

  // Terminal Node Detection
  if (in_check && legal_moves_played == 0) {
    return -100000;
  }

  return alpha;
}

// 2. NEGAMAX ALPHA-BETA (Internal Helper - Static Linkage)
static int alpha_beta(Board &board, int depth, int alpha, int beta) {
  // 1. TT PROBE
  int tt_score = 0;
  int tt_move = 0;
  if (probe_tt(board.hash_key, depth, alpha, beta, tt_score, tt_move)) {
    return tt_score;
  }

  if (depth == 0) {
    return quiescence(alpha, beta, board);
  }

  MoveList moves = generate_legal_moves(board);
  if (moves.count == 0) {
    if (is_king_in_check(board)) {
      return -100000 + depth;
    }
    return 0;
  }

  // 2. MOVE ORDERING (Prioritize TT move if available)
  sort_moves(board, moves, tt_move);

  int original_alpha = alpha;
  int best_move = 0;

  for (int i = 0; i < moves.count; ++i) {
    Move move = moves.moves[i];
    board.make_move(move);
    int score = -alpha_beta(board, depth - 1, -beta, -alpha);
    board.unmake_move(move);

    if (score >= beta) {
      // 3. STORE TT (BETA CUTOFF)
      store_tt(board.hash_key, depth, beta, TT_BETA, move);
      return beta;
    }
    if (score > alpha) {
      alpha = score;
      best_move = move;
    }
  }

  // 4. STORE TT (EXACT or ALPHA)
  int flag = (alpha <= original_alpha) ? TT_ALPHA : TT_EXACT;
  store_tt(board.hash_key, depth, alpha, flag, best_move);

  return alpha;
}

// 3. ROOT SEARCH (Public Interface - External Linkage)
void search_position(Board &board, int depth) {
  int alpha = -500000;
  int beta = 500000;

  MoveList moves = generate_legal_moves(board);
  if (moves.count == 0) {
    best_move_found = 0;
    return;
  }

  // Probe root for TT move to prioritize it immediately
  int dummy_score;
  int tt_move = 0;
  probe_tt(board.hash_key, depth, alpha, beta, dummy_score, tt_move);

  sort_moves(board, moves, tt_move);

  Move best_move_so_far = moves.moves[0];
  for (int i = 0; i < moves.count; ++i) {
    Move move = moves.moves[i];
    board.make_move(move);
    int score = -alpha_beta(board, depth - 1, -beta, -alpha);
    board.unmake_move(move);

    if (score > alpha) {
      alpha = score;
      best_move_so_far = move;
    }
  }

  best_move_found = best_move_so_far;
  // Also record the final absolute best move chosen at root into TT
  store_tt(board.hash_key, depth, alpha, TT_EXACT, best_move_so_far);
}

uint16_t get_best_move_found() { return best_move_found; }
