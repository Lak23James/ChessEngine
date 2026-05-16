// ============================================================================
// UCI.cpp — Universal Chess Interface Communication Layer
// ============================================================================
// This file handles ALL string parsing and standard I/O for the UCI protocol.
// It contains ZERO chess logic. Every point where the engine's physical state
// must be touched is marked with a // TODO: comment.
// ============================================================================

#include "uci.h"
#include "bitboard.h"
#include "movesgen.h"
#include "search.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

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

static int uci_square_index(const std::string& square) {
    if (square.size() != 2) return -1;
    int file = square[0] - 'a';
    int rank = square[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return rank * 8 + file;
}

static bool promotion_matches(uint16_t move, char promotion_char) {
    int flags = get_move_flags(move);
    if (promotion_char == '\0') {
        return flags < PROMOTION_KNIGHT;
    }
    switch (promotion_char) {
        case 'q': return flags == PROMOTION_QUEEN || flags == PROMOTION_CAPTURE_QUEEN;
        case 'r': return flags == PROMOTION_ROOK   || flags == PROMOTION_CAPTURE_ROOK;
        case 'b': return flags == PROMOTION_BISHOP || flags == PROMOTION_CAPTURE_BISHOP;
        case 'n': return flags == PROMOTION_KNIGHT || flags == PROMOTION_CAPTURE_KNIGHT;
        default:  return false;
    }
}

static std::string uci_move_to_string(uint16_t move) {
    static const char file_chars[] = "abcdefgh";
    int from = get_move_from(move);
    int to = get_move_to(move);
    std::string text;
    text += file_chars[from % 8];
    text += char('1' + (from / 8));
    text += file_chars[to % 8];
    text += char('1' + (to / 8));

    int flags = get_move_flags(move);
    switch (flags) {
        case PROMOTION_KNIGHT:
        case PROMOTION_CAPTURE_KNIGHT: text += 'n'; break;
        case PROMOTION_BISHOP:
        case PROMOTION_CAPTURE_BISHOP: text += 'b'; break;
        case PROMOTION_ROOK:
        case PROMOTION_CAPTURE_ROOK:   text += 'r'; break;
        case PROMOTION_QUEEN:
        case PROMOTION_CAPTURE_QUEEN:  text += 'q'; break;
        default: break;
    }
    return text;
}

static uint16_t parse_uci_move(Board& board, const std::string& move_string) {
    if (move_string.size() < 4) return 0;
    int from_square = uci_square_index(move_string.substr(0, 2));
    int to_square   = uci_square_index(move_string.substr(2, 2));
    char promotion_char = move_string.size() == 5 ? move_string[4] : '\0';
    if (from_square < 0 || to_square < 0) return 0;

    MoveList list;
    generate_moves(board, list);
    for (int i = 0; i < list.count; i++) {
        uint16_t candidate = list.moves[i];
        if (get_move_from(candidate) != from_square || get_move_to(candidate) != to_square) {
            continue;
        }
        if (!promotion_matches(candidate, promotion_char)) {
            continue;
        }
        return candidate;
    }
    return 0;
}

static std::string get_random_legal_move(Board& board) {
    MoveList list;
    generate_moves(board, list);

    std::vector<uint16_t> legal_moves;
    for (int i = 0; i < list.count; i++) {
        uint16_t move = list.moves[i];
        if (board.make_move(move)) {
            legal_moves.push_back(move);
            board.unmake_move(move);
        }
    }

    if (legal_moves.empty()) {
        return "";
    }

    uint16_t chosen = legal_moves[std::rand() % legal_moves.size()];
    return uci_move_to_string(chosen);
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
void UCI::parse_position(Board& board, const std::string& command) {
    std::vector<std::string> tokens = tokenize(command);
    if (tokens.size() < 2) return;

    // 1. Reset the Board First
    if (tokens[1] == "startpos") {
        board.FEN(START_FEN);
    } else if (tokens[1] == "fen") {
        std::string fen;
        size_t token_index = 2;
        // Collect all parts of the FEN until we hit "moves" or end of command
        while (token_index < tokens.size() && tokens[token_index] != "moves") {
            if (!fen.empty()) fen += ' ';
            fen += tokens[token_index];
            token_index++;
        }
        board.FEN(fen);
    }

    // 2 & 3. Locate "moves" keyword and Parse Move Stream
    size_t moves_start = 0;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == "moves") {
            moves_start = i + 1;
            break;
        }
    }

    // 4. Convert and Execute
    if (moves_start > 0) {
        for (size_t i = moves_start; i < tokens.size(); i++) {
            std::string move_string = tokens[i];
            uint16_t move = parse_uci_move(board, move_string);
            if (move != 0) {
                // Apply move permanently to the game state
                board.make_move(move);
            }
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
void UCI::parse_go(Board& board, const std::string& command) {
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

   // If no depth was specified, set a safe limit
if (depth == -1) {
    if (infinite) {
        depth = 4; // True infinite needs iterative deepening, but depth 7 is a safe ceiling for now!
    } else {
        depth = 5; // Default blitz depth
    }
}

search_position(board, depth);
uint16_t best_move = get_best_move_found();
if (best_move == 0) {
    std::cout << "bestmove 0000" << std::endl;
} else {
    std::cout << "bestmove " << uci_move_to_string(best_move) << std::endl;
}}

// ============================================================================
// loop — The main UCI event loop
// ============================================================================
// Reads from std::cin line by line and dispatches commands.
// This function blocks forever until "quit" is received.
// ============================================================================
void UCI::loop(Board& board) {
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

            else if (command_word == "isready") {
            std::cout << "readyok" << std::endl;
        }

        else if (command_word == "ucinewgame") {
            board.FEN(START_FEN);
        }

        else if (command_word == "position") {
            parse_position(board, line);
        }

        // ===== go =====
        else if (command_word == "go") {
            parse_go(board, line);
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
