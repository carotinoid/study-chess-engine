#include "Piece.h"
#include "Board.h"
#include <algorithm>

// Piece
Piece::Piece(Color color, PieceType type, Square pos) : pieceColor(color), pieceType(type), position(pos) {}
Color Piece::getColor() const { return pieceColor; }
PieceType Piece::getType() const { return pieceType; }
Square Piece::getPosition() const { return position; }
void Piece::setPosition(Square newPos) { position = newPos; }

void Piece::addSlidingMoves(std::vector<Move>& moves, const Board& board, const std::vector<std::pair<int, int>>& directions) const {
    for (const auto& dir : directions) {
        Square current = position;
        while (true) {
            current.rank += dir.first;
            current.file += dir.second;
            if (!current.isValid()) break;
            const Piece* targetPiece = board.getPieceAt(current);
            if (targetPiece == nullptr) {
                moves.push_back({position, current});
            } else {
                if (targetPiece->getColor() != this->getColor()) {
                    moves.push_back({position, current});
                }
                break;
            }
        }
    }
}

// Pawn
Pawn::Pawn(Color color, Square pos) : Piece(color, PieceType::PAWN, pos) {}
std::unique_ptr<Piece> Pawn::clone() const { return std::make_unique<Pawn>(*this); }
wchar_t Pawn::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2659' : L'\u265F'; }
std::vector<Move> Pawn::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    int direction = (pieceColor == Color::WHITE) ? 1 : -1;
    int startRank = (pieceColor == Color::WHITE) ? 1 : 6;
    int promotionRank = (pieceColor == Color::WHITE) ? 7 : 0;

    Square one_step{position.rank + direction, position.file};
    if (one_step.isValid() && board.getPieceAt(one_step) == nullptr) {
        if (one_step.rank == promotionRank) {
            moves.push_back({position, one_step, PieceType::QUEEN});
            moves.push_back({position, one_step, PieceType::ROOK});
            moves.push_back({position, one_step, PieceType::BISHOP});
            moves.push_back({position, one_step, PieceType::KNIGHT});
        } else {
            moves.push_back({position, one_step});
        }

        if (position.rank == startRank) {
            Square two_steps{position.rank + 2 * direction, position.file};
            if (two_steps.isValid() && board.getPieceAt(two_steps) == nullptr) {
                moves.push_back({position, two_steps});
            }
        }
    }

    for (int df = -1; df <= 1; df += 2) {
        Square capture_target{position.rank + direction, position.file + df};
        if (capture_target.isValid()) {
            const Piece* targetPiece = board.getPieceAt(capture_target);
            if (targetPiece != nullptr && targetPiece->getColor() != pieceColor) {
                 if (capture_target.rank == promotionRank) {
                    moves.push_back({position, capture_target, PieceType::QUEEN});
                    moves.push_back({position, capture_target, PieceType::ROOK});
                    moves.push_back({position, capture_target, PieceType::BISHOP});
                    moves.push_back({position, capture_target, PieceType::KNIGHT});
                } else {
                    moves.push_back({position, capture_target});
                }
            }
        }
    }

    std::optional<Square> enPassantTarget = board.getEnPassantTarget();
    if (enPassantTarget.has_value()) {
        if (enPassantTarget->rank == position.rank + direction &&
            (enPassantTarget->file == position.file - 1 || enPassantTarget->file == position.file + 1)) {
            moves.push_back({position, enPassantTarget.value()});
        }
    }
    return moves;
}

// Knight
Knight::Knight(Color color, Square pos) : Piece(color, PieceType::KNIGHT, pos) {}
std::unique_ptr<Piece> Knight::clone() const { return std::make_unique<Knight>(*this); }
wchar_t Knight::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2658' : L'\u265E'; }
std::vector<Move> Knight::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> knight_moves = {{1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};
    for (const auto& move : knight_moves) {
        Square target{position.rank + move.first, position.file + move.second};
        if (target.isValid()) {
            const Piece* targetPiece = board.getPieceAt(target);
            if (targetPiece == nullptr || targetPiece->getColor() != this->getColor()) {
                moves.push_back({position, target});
            }
        }
    }
    return moves;
}

// Bishop
Bishop::Bishop(Color color, Square pos) : Piece(color, PieceType::BISHOP, pos) {}
std::unique_ptr<Piece> Bishop::clone() const { return std::make_unique<Bishop>(*this); }
wchar_t Bishop::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2657' : L'\u265D'; }
std::vector<Move> Bishop::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    addSlidingMoves(moves, board, directions);
    return moves;
}

// Rook
Rook::Rook(Color color, Square pos) : Piece(color, PieceType::ROOK, pos) {}
std::unique_ptr<Piece> Rook::clone() const { return std::make_unique<Rook>(*this); }
wchar_t Rook::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2656' : L'\u265C'; }
std::vector<Move> Rook::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    addSlidingMoves(moves, board, directions);
    return moves;
}

// Queen
Queen::Queen(Color color, Square pos) : Piece(color, PieceType::QUEEN, pos) {}
std::unique_ptr<Piece> Queen::clone() const { return std::make_unique<Queen>(*this); }
wchar_t Queen::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2655' : L'\u265B'; }
std::vector<Move> Queen::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    addSlidingMoves(moves, board, directions);
    return moves;
}

// King
King::King(Color color, Square pos) : Piece(color, PieceType::KING, pos) {}
std::unique_ptr<Piece> King::clone() const { return std::make_unique<King>(*this); }
wchar_t King::getUnicodeChar() const { return (pieceColor == Color::WHITE) ? L'\u2654' : L'\u265A'; }
std::vector<Move> King::getPossibleMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> king_moves = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (const auto& move : king_moves) {
        Square target{position.rank + move.first, position.file + move.second};
        if (target.isValid()) {
            const Piece* targetPiece = board.getPieceAt(target);
            if (targetPiece == nullptr || targetPiece->getColor() != this->getColor()) {
                moves.push_back({position, target});
            }
        }
    }

    CastleRights rights = board.getCastleRights();
    Color opponentColor = (pieceColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    if (!board.isKingInCheck(pieceColor)) {
        if (pieceColor == Color::WHITE) {
            if (rights.whiteKingSide && !board.getPieceAt({0, 5}) && !board.getPieceAt({0, 6}) && !board.isSquareAttackedBy({0, 5}, opponentColor) && !board.isSquareAttackedBy({0, 6}, opponentColor)) {
                moves.push_back({position, {0, 6}});
            }
            if (rights.whiteQueenSide && !board.getPieceAt({0, 1}) && !board.getPieceAt({0, 2}) && !board.getPieceAt({0, 3}) && !board.isSquareAttackedBy({0, 3}, opponentColor) && !board.isSquareAttackedBy({0, 2}, opponentColor)) {
                moves.push_back({position, {0, 2}});
            }
        } else {
            if (rights.blackKingSide && !board.getPieceAt({7, 5}) && !board.getPieceAt({7, 6}) && !board.isSquareAttackedBy({7, 5}, opponentColor) && !board.isSquareAttackedBy({7, 6}, opponentColor)) {
                moves.push_back({position, {7, 6}});
            }
            if (rights.blackQueenSide && !board.getPieceAt({7, 1}) && !board.getPieceAt({7, 2}) && !board.getPieceAt({7, 3}) && !board.isSquareAttackedBy({7, 3}, opponentColor) && !board.isSquareAttackedBy({7, 2}, opponentColor)) {
                moves.push_back({position, {7, 2}});
            }
        }
    }
    return moves;
}