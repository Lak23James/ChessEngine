#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <cctype>
#include "type.h"

// WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5, BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
// 0 to 63 for squares A1 to H8 0-=> A1, 1=>B1, ..., 63=>H8
const char ascii_pieces[13] = "PNBRQKpnbrqk";

constexpr uint64_t RANK_2 = 0x000000000000FF00ULL;
constexpr uint64_t RANK_4 = 0x00000000FF000000ULL;
constexpr uint64_t RANK_5 = 0x000000FF00000000ULL;
constexpr uint64_t RANK_7 = 0x00FF000000000000ULL;

class Board {
public:
    static constexpr uint64_t FILE_A  = 0x0101010101010101ULL;
    static constexpr uint64_t FILE_H  = 0x8080808080808080ULL;
    static constexpr uint64_t FILE_GH = 0xC0C0C0C0C0C0C0C0ULL;
    static constexpr uint64_t FILE_AB = 0x0303030303030303ULL;

    uint64_t knight_attacks[64];
    uint64_t king_attacks[64];
    uint64_t rook_attacks[64];
    uint64_t bishop_attacks[64];
    uint64_t queen_attacks[64];
    uint64_t pawn_attacks_white[64];
    uint64_t pawn_attacks_black[64];
   
    
    // Color Occupancy Getters
    inline uint64_t get_white_pieces() const  { return occupied[WHITE]; }
    inline uint64_t get_black_pieces() const  { return occupied[BLACK]; }
    
    // API for getting a particular bitboard
    inline uint64_t get_white_bishops() const { return bitboard[WB]; }
    inline uint64_t get_white_rooks() const   { return bitboard[WR]; }
    inline uint64_t get_white_queens() const  { return bitboard[WQ]; }
    inline uint64_t get_white_knights() const { return bitboard[WN]; }
    inline uint64_t get_white_kings() const   { return bitboard[WK]; }
    inline uint64_t get_white_pawns() const {return bitboard[WP];}
    inline uint64_t get_black_bishops() const { return bitboard[BB]; }
    inline uint64_t get_black_rooks() const   { return bitboard[BR]; }
    inline uint64_t get_black_queens() const  { return bitboard[BQ]; }
    inline uint64_t get_black_knights() const { return bitboard[BN]; }
    inline uint64_t get_black_kings() const   { return bitboard[BK]; }
    inline uint64_t get_black_pawns() const {return bitboard[BP]; }
    // Returns a bitboard of every piece currently on the board
    inline uint64_t get_occupied() const {return occupied[BOTH]; }
    // Returns a bitboard of all empty squares
    inline uint64_t get_empty() const { return ~occupied[BOTH];}

    // Public setter for bitboards
    void add_to_bitboard(int index, uint64_t mask) {
        bitboard[index] |= mask;
        occupied[index < 6 ? WHITE : BLACK] |= mask;
        occupied[BOTH] |= mask;
    }

    // Function to print the board
    void print_board();

    void print_attacks(uint64_t mask);

    // The FEN Parser
    void FEN(const std::string& fen);

    // Function to initialize knight attacks
    void init_knights();

    // Function for moves of the king
    void init_kings();

    // Function to initialize rook attacks
    void init_rooks(uint64_t block, int rook_square);

    // Function to initialize bishop attacks
    void init_bishops(uint64_t block, int square);

    // Function to initialize queen attacks
    // A queen's attacks are just the combination of rook and bishop attacks
    void init_queens(uint64_t block, int square);

    // Function to initialize pawn moves
    void init_pawn_attacks();

private:
    // 0-5 for white pieces, 6-11 for black pieces {Representation of pieces}:
    // WP=0, WN=1, WB=2, WR=3, WQ=4, WK=5, BP=6, BN=7, BB=8, BR=9, BQ=10, BK=11
    uint64_t bitboard[12] = {0};
    // 0 for white pieces, 1 for black pieces, 2 for double occupancy
    uint64_t occupied[3] = {0};
};