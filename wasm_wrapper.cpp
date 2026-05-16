#include <emscripten.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include "bitboard.h"
#include "movesgen.h"
#include "zobrist.h"
#include "move.h"

static bool is_initialized = false;
static char move_buffer[10];

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* get_random_legal_move(const char* fen) {
        // 1. One-time initialization
        if (!is_initialized) {
            init_zobrist();
            std::srand(std::time(nullptr));
            is_initialized = true;
        }

        // 2. Set up the board
        Board board;
        board.init_kings();
        board.init_knights();
        board.init_pawn_attacks();
        
        board.FEN(std::string(fen));

        // 3. Generate pseudo-legal moves
        MoveList list;
        generate_moves(board, list);

        // 4. Filter for strictly legal moves
        std::vector<uint16_t> legal_moves;
        for (int i = 0; i < list.count; i++) {
            uint16_t move = list.moves[i];
            if (board.make_move(move)) {
                legal_moves.push_back(move);
                board.unmake_move(move); // Undo the move
            }
        }

        if (legal_moves.empty()) {
            return "none"; // Checkmate or stalemate
        }

        // 5. Pick a random legal move
        int random_index = std::rand() % legal_moves.size();
        uint16_t chosen_move = legal_moves[random_index];

        // 6. Convert to UCI string notation (e.g. "e2e4")
        int from = get_move_from(chosen_move);
        int to = get_move_to(chosen_move);
        int flags = get_move_flags(chosen_move);

        static const char file_chars[] = "abcdefgh";
        std::string result = "";
        result += file_chars[from % 8];
        result += std::to_string((from / 8) + 1);
        result += file_chars[to % 8];
        result += std::to_string((to / 8) + 1);

        switch (flags) {
            case PROMOTION_KNIGHT: case PROMOTION_CAPTURE_KNIGHT: result += 'n'; break;
            case PROMOTION_BISHOP: case PROMOTION_CAPTURE_BISHOP: result += 'b'; break;
            case PROMOTION_ROOK:   case PROMOTION_CAPTURE_ROOK:   result += 'r'; break;
            case PROMOTION_QUEEN:  case PROMOTION_CAPTURE_QUEEN:  result += 'q'; break;
        }

        // 7. Copy to static buffer and return to JS
        std::strncpy(move_buffer, result.c_str(), sizeof(move_buffer));
        return move_buffer;
    }
}
