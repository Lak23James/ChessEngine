enum TTFlag {
    TT_EXACT, // We searched the whole node. This is the exact evaluation.
    TT_ALPHA, // Upperbound (Failed low - this position is bad for us)
    TT_BETA   // Lowerbound (Failed high - this position caused a cutoff)
};
struct TTEntry {
    uint64_t key;      // The Zobrist hash (to verify we are looking at the right board)
    int score;         // The evaluation score (+450, -120, etc.)
    int depth;         // How deep the engine searched to get this score
    int flag;          // TT_EXACT, TT_ALPHA, or TT_BETA
    int best_move;     // The integer representing the best move found here
};
const int TT_SIZE = 1048576;
extern TTEntry tt[TT_SIZE]; 