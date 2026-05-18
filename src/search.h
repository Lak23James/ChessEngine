#pragma once

#include <cstdint>
#include "bitboard.h"
#include <chrono>

extern std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
extern int allocated_time_ms;
extern bool time_is_up;
extern long long nodes_searched;

void clear_tt();
void check_time();
int alpha_beta(Board& board, int depth, int alpha, int beta);
void search_position(Board& board, int max_time_ms);
uint16_t get_best_move_found();

