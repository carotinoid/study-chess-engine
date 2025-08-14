#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Bitboard.h"
#include "DataTypes.h"
#include <vector>

namespace MoveGen {

    void generateMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateCaptureMoves(const BoardState& boardState, std::vector<Move>& moves);

    // Functions for generating moves for each piece type
    void generatePawnMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateKnightMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateBishopMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateRookMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateQueenMoves(const BoardState& boardState, std::vector<Move>& moves);
    void generateKingMoves(const BoardState& boardState, std::vector<Move>& moves);

}

#endif // MOVEGEN_H
