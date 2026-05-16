// ============================================================================
// UCI.cpp — Universal Chess Interface Communication Layer
// ============================================================================
// This file handles ALL string parsing and standard I/O for the UCI protocol.
// It contains ZERO chess logic. Every point where the engine's physical state
// must be touched is marked with a // TODO: comment.
// ============================================================================

#include "uci.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ============================================================================
// Constants
// ============================================================================
static const std::string ENGINE_NAME    = "ChessEngine";
static const std::string ENGINE_AUTHOR  = "Lak23James";
static const std::string START_FEN      = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// ============================================================================
// Helper: split a string into tokens by whitespace
// ============================================================================
static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// ============================================================================
// parse_position
// ============================================================================
// Handles two forms:
//   position startpos
//   position startpos moves e2e4 e7e5 ...
//   position fen <fen_string>
//   position fen <fen_string> moves e2e4 e7e5 ...
// ============================================================================
void UCI::parse_position(const std::string& command) {
    std::vector<std::string> tokens = tokenize(command);
    if (tokens.size() < 2) return;

    size_t moves_index = 0;  // index where "moves" token appears (0 = not found)

    if (tokens[1] == "startpos") {
        // ------------------------------------------------------------------
        // TODO: Reset board to starting position
        // board.FEN(START_FEN);
        // ------------------------------------------------------------------
        moves_index = 2;  // "moves" would be at index 2 if present

    } else if (tokens[1] == "fen") {
        // Reconstruct the FEN string from tokens[2..7]
        // A full FEN has 6 fields: pieces, side, castling, ep, halfmove, fullmove
        std::string fen = "";
        size_t fen_end = 2;
        for (size_t i = 2; i < tokens.size() && i < 8; i++) {
            if (tokens[i] == "moves") break;
            if (!fen.empty()) fen += " ";
            fen += tokens[i];
            fen_end = i + 1;
        }

        // ------------------------------------------------------------------
        // TODO: Set up board from the parsed FEN
        // board.FEN(fen);
        // ------------------------------------------------------------------

        moves_index = fen_end;  // "moves" token would be here
    }

    // Now look for the "moves" token and apply each move
    if (moves_index < tokens.size() && tokens[moves_index] == "moves") {
        for (size_t i = moves_index + 1; i < tokens.size(); i++) {
            std::string move_string = tokens[i];

            // --------------------------------------------------------------
            // TODO: Parse the move string and apply it to the board.
            //
            // move_string is in UCI long algebraic notation:
            //   "e2e4", "e7e5", "e1g1" (castling), "e7e8q" (promotion)
            //
            // You need a helper function that:
            //   1. Extracts from_square (e.g., "e2" -> file=4, rank=1 -> sq=12)
            //   2. Extracts to_square   (e.g., "e4" -> file=4, rank=3 -> sq=28)
            //   3. Detects promotion char if present (5th character: q/r/b/n)
            //   4. Finds the matching move in the legal move list
            //   5. Calls board.make_move(matched_move)
            //
            // Example implementation:
            //   uint16_t parsed = parse_uci_move(board, move_string);
            //   board.make_move(parsed);
            // --------------------------------------------------------------
        }
    }
}

// ============================================================================
// parse_go
// ============================================================================
// Handles search parameters. For now, we only parse "depth".
// Full UCI go parameters for future expansion:
//   go depth 5
//   go wtime 300000 btime 300000 winc 0 binc 0
//   go movetime 5000
//   go infinite
//   go nodes 1000000
// ============================================================================
void UCI::parse_go(const std::string& command) {
    std::vector<std::string> tokens = tokenize(command);

    int depth      = -1;  // -1 means not specified
    int movetime   = -1;
    int wtime      = -1;
    int btime      = -1;
    int winc       = -1;
    int binc       = -1;
    int movestogo  = -1;
    bool infinite  = false;

    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth"     && i + 1 < tokens.size()) depth     = std::stoi(tokens[++i]);
        if (tokens[i] == "movetime"  && i + 1 < tokens.size()) movetime  = std::stoi(tokens[++i]);
        if (tokens[i] == "wtime"     && i + 1 < tokens.size()) wtime     = std::stoi(tokens[++i]);
        if (tokens[i] == "btime"     && i + 1 < tokens.size()) btime     = std::stoi(tokens[++i]);
        if (tokens[i] == "winc"      && i + 1 < tokens.size()) winc      = std::stoi(tokens[++i]);
        if (tokens[i] == "binc"      && i + 1 < tokens.size()) binc      = std::stoi(tokens[++i]);
        if (tokens[i] == "movestogo" && i + 1 < tokens.size()) movestogo = std::stoi(tokens[++i]);
        if (tokens[i] == "infinite") infinite = true;
    }

    // If no depth was given, use a sensible default
    if (depth == -1 && !infinite) {
        depth = 6;  // Default search depth — adjust as engine matures
    }

    // ------------------------------------------------------------------
    // TODO: Call your search function here.
    //
    // Example:
    //   uint16_t best_move = search(board, depth);
    //
    // After the search completes, you MUST print the best move in UCI
    // long algebraic notation:
    //
    //   std::cout << "bestmove " << move_to_uci_string(best_move) << std::endl;
    //
    // During search, you should also print "info" lines:
    //   std::cout << "info depth " << d
    //             << " score cp " << eval
    //             << " nodes " << nodes
    //             << " pv " << pv_string << std::endl;
    // ------------------------------------------------------------------

    // Placeholder output so the GUI doesn't hang:
    std::cout << "info string search not implemented yet" << std::endl;
    std::cout << "bestmove e2e4" << std::endl;
}

// ============================================================================
// loop — The main UCI event loop
// ============================================================================
// Reads from std::cin line by line and dispatches commands.
// This function blocks forever until "quit" is received.
// ============================================================================
void UCI::loop() {
    std::string line;

    // Print the engine identity on startup (some GUIs expect this immediately)
    // Uncomment the next 3 lines if your GUI sends "uci" automatically:
    // std::cout << "id name " << ENGINE_NAME << std::endl;
    // std::cout << "id author " << ENGINE_AUTHOR << std::endl;
    // std::cout << "uciok" << std::endl;

    while (std::getline(std::cin, line)) {
        // Strip trailing whitespace / carriage return (Windows compatibility)
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty()) continue;

        // Extract the first word to identify the command
        std::string command_word;
        {
            std::istringstream iss(line);
            iss >> command_word;
        }

        // ===== uci =====
        if (command_word == "uci") {
            std::cout << "id name ChessEngine v1.0" << std::endl;
            std::cout << "id author Lakshya" << std::endl;

            // TODO: Print any configurable options here using "option" command
            // Example:
            // std::cout << "option name Hash type spin default 64 min 1 max 1024" << std::endl;
            // std::cout << "option name Threads type spin default 1 min 1 max 16" << std::endl;

            std::cout << "uciok" << std::endl;
        }

        // ===== isready =====
        else if (command_word == "isready") {
            // The engine must respond "readyok" after finishing any pending work.
            // Since we have no async tasks yet, respond immediately.
            std::cout << "readyOk" << std::endl;
        }

       if (command == "ucinewgame") {
    // We will clear the Hash Tables here later when we build them.
    // For now, just reset the board to the starting position.
    
   
    board.parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); 
}

        // ===== position =====
        else if (command_word == "position") {
            parse_position(line);
        }

        // ===== go =====
        else if (command_word == "go") {
            parse_go(line);
        }

        // ===== stop =====
        else if (command_word == "stop") {
            // ----------------------------------------------------------
            // TODO: Signal the search thread to stop immediately.
            //   search_info.stopped = true;
            // The search function should check this flag periodically.
            // ----------------------------------------------------------
        }

        // ===== quit =====
        else if (command_word == "quit") {
            break;
        }

        // ===== debug / unknown =====
        else {
            // Silently ignore unknown commands (UCI spec requires this)
        }
    }
}
