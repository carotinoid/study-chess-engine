#ifndef DATATYPES_H
#define DATATYPES_H

#include <optional>
#include <string>
#include <iostream>

enum class Color { WHITE, BLACK };
enum class PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

struct CastleRights {
    bool whiteKingSide = true;
    bool whiteQueenSide = true;
    bool blackKingSide = true;
    bool blackQueenSide = true;
};

struct Square {
    int rank;
    int file;

    bool operator==(const Square& other) const {
        return rank == other.rank && file == other.file;
    }
    bool isValid() const {
        return rank >= 0 && rank < 8 && file >= 0 && file < 8;
    }
    std::string toString() const {
        return std::string(1, (char)('a' + file)) + std::string(1, (char)('1' + rank));
    }
}; 

struct Move {
    Square start;
    Square end;
    std::optional<PieceType> promotionPiece = std::nullopt;

    bool operator==(const Move& other) const {
        return start == other.start && end == other.end && promotionPiece == other.promotionPiece;
    }
    std::string toString() const {
        std::string str = start.toString() + end.toString();
        if (promotionPiece) {
            switch (*promotionPiece) {
                case PieceType::QUEEN: str += 'q'; break;
                case PieceType::ROOK: str += 'r'; break;
                case PieceType::BISHOP: str += 'b'; break;
                case PieceType::KNIGHT: str += 'n'; break;
                default: break;
            }
        }
        return str;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Move& move) {
    os << move.toString();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Color& color) {
    os << (color == Color::WHITE ? "WHITE" : "BLACK");
    return os;
}

inline std::string to_string(PieceType p) {
    switch (p) {
        case PieceType::PAWN:   return "P";
        case PieceType::KNIGHT: return "N";
        case PieceType::BISHOP: return "B";
        case PieceType::ROOK:   return "R";
        case PieceType::QUEEN:  return "Q";
        case PieceType::KING:   return "K";
        default: return "";
    }
}

#endif // DATATYPES_H