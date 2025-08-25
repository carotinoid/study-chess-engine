#ifndef GAME_H
#define GAME_H

#include <vector>
#include "Bitboard.h"

enum class GameStatus { ONGOING, CHECKMATE, STALEMATE, DRAW };

class Game {
public:
    Game();
    Game(const std::string& fen);

    BitboardRepresentation& getBoard();
    const BitboardRepresentation& getBoard() const;
    GameStatus getStatus() const;

    std::vector<Move> getLegalMovesForPieceAt(Square s) const;
    bool makeMove(const Move& move);
    void makeNullMove();
    std::vector<Move> generateAllLegalMoves() const;

private:
    void updateStatus();
    BitboardRepresentation board;
    GameStatus status;
    std::vector<Move> moveHistory;
};

#endif // GAME_H