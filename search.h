#pragma once

#include <cstdint>
#include "bitboard.h"

void search_position(Board& board, int depth);
uint16_t get_best_move_found();
