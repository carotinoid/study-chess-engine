#include "../include/Debug.h"
#include <iostream>
#include <vector>
#include <optional>

namespace {
    // Helper to get piece type and color at a square
    auto getPieceAt(const BoardState& boardState, int rank, int file) -> std::optional<std::pair<PieceType, Color>> {
        Bitboard mask = 1ULL << (rank * 8 + file);
        if (boardState.pawn[0] & mask) return {{PieceType::PAWN, Color::WHITE}};
        if (boardState.pawn[1] & mask) return {{PieceType::PAWN, Color::BLACK}};
        if (boardState.knight[0] & mask) return {{PieceType::KNIGHT, Color::WHITE}};
        if (boardState.knight[1] & mask) return {{PieceType::KNIGHT, Color::BLACK}};
        if (boardState.bishop[0] & mask) return {{PieceType::BISHOP, Color::WHITE}};
        if (boardState.bishop[1] & mask) return {{PieceType::BISHOP, Color::BLACK}};
        if (boardState.rook[0] & mask) return {{PieceType::ROOK, Color::WHITE}};
        if (boardState.rook[1] & mask) return {{PieceType::ROOK, Color::BLACK}};
        if (boardState.queen[0] & mask) return {{PieceType::QUEEN, Color::WHITE}};
        if (boardState.queen[1] & mask) return {{PieceType::QUEEN, Color::BLACK}};
        if (boardState.king[0] & mask) return {{PieceType::KING, Color::WHITE}};
        if (boardState.king[1] & mask) return {{PieceType::KING, Color::BLACK}};
        return std::nullopt;
    };

    // Helper to get character for a piece
    auto getPieceChar(PieceType type, Color color) -> char {
        switch (type) {
            case PieceType::PAWN:   return (color == Color::WHITE) ? 'P' : 'p';
            case PieceType::KNIGHT: return (color == Color::WHITE) ? 'N' : 'n';
            case PieceType::BISHOP: return (color == Color::WHITE) ? 'B' : 'b';
            case PieceType::ROOK:   return (color == Color::WHITE) ? 'R' : 'r';
            case PieceType::QUEEN:  return (color == Color::WHITE) ? 'Q' : 'q';
            case PieceType::KING:   return (color == Color::WHITE) ? 'K' : 'k';
        }
        return ' ';
    };
}

void Debug::renderBoard(const Game& game) {
    const BitboardRepresentation& board = game.getBoard();
    const BoardState& boardState = board.getState();

    std::cout << "\n   a b c d e f g h\n";
    std::cout << " +-----------------+\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << r + 1 << "|";
        for (int f = 0; f < 8; ++f) {
            auto piece = getPieceAt(boardState, r, f);
            if (piece) {
                std::cout << " " << getPieceChar(piece->first, piece->second);
            } else {
                std::cout << " .";
            }
        }
        std::cout << " |" << r + 1 << "\n";
    }
    std::cout << " +-----------------+\n";
    std::cout << "   a b c d e f g h\n\n";
}
