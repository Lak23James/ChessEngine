#pragma once

#include <string>

namespace UCI {
    // Main UCI loop — listens on stdin forever until "quit"
    void loop();

    // Parses "position startpos moves ..." or "position fen ... moves ..."
    void parse_position(const std::string& command);

    // Parses "go depth X" and other go parameters
    void parse_go(const std::string& command);
}
