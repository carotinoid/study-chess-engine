#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include <memory>
#include "DataTypes.h"

class Board; // Forward declaration

class Piece {
public:
    Piece(Color color, PieceType type, Square pos);
    virtual ~Piece() = default;

    virtual std::vector<Move> getPossibleMoves(const Board& board) const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;
    virtual wchar_t getUnicodeChar() const = 0;

    Color getColor() const;
    PieceType getType() const;
    Square getPosition() const;
    void setPosition(Square newPos);

protected:
    Color pieceColor;
    PieceType pieceType;
    Square position;
    void addSlidingMoves(std::vector<Move>& moves, const Board& board, const std::vector<std::pair<int, int>>& directions) const;
};

class Pawn : public Piece {
public:
    Pawn(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

class Knight : public Piece {
public:
    Knight(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

class Bishop : public Piece {
public:
    Bishop(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

class Rook : public Piece {
public:
    Rook(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

class Queen : public Piece {
public:
    Queen(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

class King : public Piece {
public:
    King(Color color, Square pos);
    std::vector<Move> getPossibleMoves(const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    wchar_t getUnicodeChar() const override;
};

#endif // PIECE_H