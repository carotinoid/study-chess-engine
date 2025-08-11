#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "Game.h"
#include <array>

class AIPlayer {
public:
    Move findBestMove(Game& game, int depth);

private:
    int minimax(Game& game, int depth, bool maximizingPlayer, int alpha, int beta);
    int evaluate(const Board& board);

    static const std::array<int, 64> pawn_pst;
    static const std::array<int, 64> knight_pst;
    static const std::array<int, 64> bishop_pst;
    static const std::array<int, 64> rook_pst;
    static const std::array<int, 64> queen_pst;
    static const std::array<int, 64> king_pst_mid;
};

#endif // AIPLAYER_H