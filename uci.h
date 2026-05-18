#pragma once

#include <string>
#include "bitboard.h"

namespace UCI {
    // Main UCI loop — listens on stdin forever until "quit"
    void loop(Board& board);

    // Parses "position startpos moves ..." or "position fen ... moves ..."
    void parse_position(Board& board, const std::string& command);

    // Parses "go depth X" and other go parameters
    void parse_go(Board& board, const std::string& command);
}

std::string uci_move_to_string(uint16_t move);
