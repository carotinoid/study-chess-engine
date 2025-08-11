#ifndef BOARD_H
#define BOARD_H

#include <array>
#include <memory>
#include <optional>
#include "DataTypes.h"

class Piece; // Forward declaration

struct CastleRights {
    bool whiteKingSide = true;
    bool whiteQueenSide = true;
    bool blackKingSide = true;
    bool blackQueenSide = true;
};

class Board {
public:
    Board();
    Board(const Board& other);
    
    void setupInitialPosition();
    const Piece* getPieceAt(Square s) const;
    Piece* getPieceAt(Square s);
    std::unique_ptr<Piece> takePieceAt(Square s);
    void placePieceAt(std::unique_ptr<Piece> piece, Square s);
    bool isKingInCheck(Color kingColor) const;
    bool isSquareAttackedBy(Square s, Color attackerColor) const;
    Color getCurrentTurn() const;
    void switchTurn();
    CastleRights getCastleRights() const;
    void setCastleRights(CastleRights rights);
    std::optional<Square> getEnPassantTarget() const;
    void setEnPassantTarget(std::optional<Square> s);

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid;
    Color currentTurn;
    CastleRights castleRights;
    std::optional<Square> enPassantTarget;
};

#endif // BOARD_H