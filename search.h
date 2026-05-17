// ============================================================================
// search.h — Chess Engine Search Interface
// ============================================================================
// This header declares the public interface for the search algorithm (Negamax 
// with Alpha-Beta pruning) used to calculate the best move for a given position.
// ============================================================================

#pragma once

#include <cstdint>
#include "bitboard.h"

// Starts the search process on the given board position to a fixed depth.
// Once complete, the chosen move is stored and can be retrieved using get_best_move_found().
void search_position(Board& board, int depth);

// Returns the best move (encoded as a uint16_t) found during the most recent search.
uint16_t get_best_move_found();
