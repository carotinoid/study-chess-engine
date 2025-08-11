#ifndef DATATYPES_H
#define DATATYPES_H

#include <optional>

enum class Color { WHITE, BLACK };
enum class PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

struct Square {
    int rank;
    int file;

    bool operator==(const Square& other) const {
        return rank == other.rank && file == other.file;
    }
    bool isValid() const {
        return rank >= 0 && rank < 8 && file >= 0 && file < 8;
    }
};

struct Move {
    Square start;
    Square end;
    std::optional<PieceType> promotionPiece = std::nullopt;

    bool operator==(const Move& other) const {
        return start == other.start && end == other.end && promotionPiece == other.promotionPiece;
    }
};

#endif // DATATYPES_H