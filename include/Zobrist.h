#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "DataTypes.h"
#include "Bitboard.h"
#include <array>
#include <cstdint>

namespace Zobrist {

    // Zobrist keys for each piece on each square
    extern std::array<std::array<uint64_t, 64>, 12> pieceKeys;
    // Zobrist key for black to move
    extern uint64_t blackToMoveKey;
    // Zobrist keys for castling rights (indexed by CastleRights bitmask)
    extern std::array<uint64_t, 16> castleKeys;
    // Zobrist keys for en passant file (indexed by file 0-7)
    extern std::array<uint64_t, 8> enPassantKeys;

    // Initializes all Zobrist keys with random values
    void Init();

    // Computes the Zobrist hash for a given board state from scratch
    uint64_t ComputeHash(const BoardState& boardState);

} // namespace Zobrist

#endif // ZOBRIST_H
