#include "Board.h"
#include "Piece.h"
#include <algorithm>

Board::Board() : currentTurn(Color::WHITE) {
    setupInitialPosition();
}

Board::Board(const Board& other) {
    this->currentTurn = other.currentTurn;
    this->castleRights = other.castleRights;
    this->enPassantTarget = other.enPassantTarget;
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            if (other.grid[r][f]) {
                this->grid[r][f] = other.grid[r][f]->clone();
            } else {
                this->grid[r][f] = nullptr;
            }
        }
    }
}

void Board::setupInitialPosition() {
    for (auto& row : grid) for (auto& piece : row) piece = nullptr;
    auto createPiece = [](PieceType type, Color color, Square s) -> std::unique_ptr<Piece> {
        switch (type) {
            case PieceType::PAWN:   return std::make_unique<Pawn>(color, s);
            case PieceType::KNIGHT: return std::make_unique<Knight>(color, s);
            case PieceType::BISHOP: return std::make_unique<Bishop>(color, s);
            case PieceType::ROOK:   return std::make_unique<Rook>(color, s);
            case PieceType::QUEEN:  return std::make_unique<Queen>(color, s);
            case PieceType::KING:   return std::make_unique<King>(color, s);
        }
        return nullptr;
    };
    Color c = Color::WHITE;
    grid[0][0] = createPiece(PieceType::ROOK, c, {0, 0}); grid[0][1] = createPiece(PieceType::KNIGHT, c, {0, 1}); grid[0][2] = createPiece(PieceType::BISHOP, c, {0, 2}); grid[0][3] = createPiece(PieceType::QUEEN, c, {0, 3}); grid[0][4] = createPiece(PieceType::KING, c, {0, 4}); grid[0][5] = createPiece(PieceType::BISHOP, c, {0, 5}); grid[0][6] = createPiece(PieceType::KNIGHT, c, {0, 6}); grid[0][7] = createPiece(PieceType::ROOK, c, {0, 7});
    for (int f = 0; f < 8; ++f) grid[1][f] = createPiece(PieceType::PAWN, c, {1, f});
    c = Color::BLACK;
    grid[7][0] = createPiece(PieceType::ROOK, c, {7, 0}); grid[7][1] = createPiece(PieceType::KNIGHT, c, {7, 1}); grid[7][2] = createPiece(PieceType::BISHOP, c, {7, 2}); grid[7][3] = createPiece(PieceType::QUEEN, c, {7, 3}); grid[7][4] = createPiece(PieceType::KING, c, {7, 4}); grid[7][5] = createPiece(PieceType::BISHOP, c, {7, 5}); grid[7][6] = createPiece(PieceType::KNIGHT, c, {7, 6}); grid[7][7] = createPiece(PieceType::ROOK, c, {7, 7});
    for (int f = 0; f < 8; ++f) grid[6][f] = createPiece(PieceType::PAWN, c, {6, f});
    currentTurn = Color::WHITE;
    castleRights = {};
    enPassantTarget = std::nullopt;
}

const Piece* Board::getPieceAt(Square s) const {
    if (!s.isValid()) return nullptr;
    return grid[s.rank][s.file].get();
}

Piece* Board::getPieceAt(Square s) {
    if (!s.isValid()) return nullptr;
    return grid[s.rank][s.file].get();
}

std::unique_ptr<Piece> Board::takePieceAt(Square s) {
    if (!s.isValid()) return nullptr;
    return std::move(grid[s.rank][s.file]);
}

void Board::placePieceAt(std::unique_ptr<Piece> piece, Square s) {
    if (!s.isValid()) return;
    if (piece) piece->setPosition(s);
    grid[s.rank][s.file] = std::move(piece);
}

bool Board::isKingInCheck(Color kingColor) const {
    Square kingPos = {-1, -1};
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const Piece* p = getPieceAt({r, f});
            if (p && p->getType() == PieceType::KING && p->getColor() == kingColor) {
                kingPos = {r, f};
                break;
            }
        }
        if (kingPos.isValid()) break;
    }
    if (!kingPos.isValid()) return false;
    return isSquareAttackedBy(kingPos, (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE);
}

bool Board::isSquareAttackedBy(Square s, Color attackerColor) const {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const Piece* p = getPieceAt({r, f});
            if (p && p->getColor() == attackerColor) {
                if (p->getType() == PieceType::KING) {
                     if (std::max(std::abs(s.rank - r), std::abs(s.file - f)) == 1) return true;
                } else {
                    std::vector<Move> moves = p->getPossibleMoves(*this);
                    for (const auto& move : moves) {
                        if (move.end == s) {
                            if (p->getType() == PieceType::PAWN) {
                                if (move.end.file != move.start.file) return true;
                            } else {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

Color Board::getCurrentTurn() const { return currentTurn; }
void Board::switchTurn() { currentTurn = (currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE; }
CastleRights Board::getCastleRights() const { return castleRights; }
void Board::setCastleRights(CastleRights rights) { castleRights = rights; }
std::optional<Square> Board::getEnPassantTarget() const { return enPassantTarget; }
void Board::setEnPassantTarget(std::optional<Square> s) { enPassantTarget = s; }