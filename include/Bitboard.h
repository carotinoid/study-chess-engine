#ifndef BITBOARD_H
#define BITBOARD_H

#include "DataTypes.h"
#include <cstdint>
#include <optional>

// Bitboard is a 64-bit integer representing the 64 squares of a chessboard.
using Bitboard = uint64_t;

// Bitboard representation of the board state
struct BoardState {
    // Bitboards for each piece type and color
    Bitboard pawn[2];
    Bitboard knight[2];
    Bitboard bishop[2];
    Bitboard rook[2];
    Bitboard queen[2];
    Bitboard king[2];

    // Bitboards for all pieces of each color
    Bitboard white_pieces;
    Bitboard black_pieces;
    Bitboard all_pieces;

    CastleRights castleRights;
    std::optional<Square> enPassantTarget;
    Color currentTurn;
    int halfmoveClock;
    int fullmoveNumber;
};

class BitboardRepresentation {
public:
    BitboardRepresentation();
    void setupInitialPosition();

    const BoardState& getState() const;

    // Functions to manipulate the board state
    void makeMove(const Move& move);
    bool isKingInCheck(Color kingColor) const;
    // void unmakeMove(...); // For search algorithms

    // Move generation
    // std::vector<Move> generateMoves();

private:
    BoardState currentState;

    void updateCompositeBitboards();
};

// Utility functions for bitboards
namespace BitboardUtils {
    bool isSquareAttackedBy(Square s, Color attackerColor, const BoardState& boardState);

    constexpr Bitboard squareToBitboard(Square s) {
        return 1ULL << (s.rank * 8 + s.file);
    }

    void printBitboard(Bitboard b);
}

#endif // BITBOARD_H
