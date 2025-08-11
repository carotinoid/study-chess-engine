#ifndef GAME_H
#define GAME_H

#include <vector>
#include "Board.h"

enum class GameStatus { ONGOING, CHECKMATE, STALEMATE, DRAW };

class Game {
public:
    Game();

    Board& getBoard();
    const Board& getBoard() const;
    GameStatus getStatus() const;

    std::vector<Move> getLegalMovesForPieceAt(Square s);
    bool makeMove(const Move& move);
    std::vector<Move> generateAllLegalMoves();

private:
    void updateStatus();
    Board board;
    GameStatus status;
    std::vector<Move> moveHistory;
};

#endif // GAME_H