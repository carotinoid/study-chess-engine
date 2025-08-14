#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "Bitboard.h"
#include "DataTypes.h"
#include <array>

class Game; // Forward Declaration

class AIPlayer {
public:
    Move findBestMove(Game& game, int depth);

private:
    int minimax(Game& game, int depth, bool maximizingPlayer, int alpha, int beta);
    int quiescenceSearch(Game& game, int alpha, int beta);
    int evaluate(const BoardState& boardState);

    static const std::array<int, 64> pawn_pst;
    static const std::array<int, 64> knight_pst;
    static const std::array<int, 64> bishop_pst;
    static const std::array<int, 64> rook_pst;
    static const std::array<int, 64> queen_pst;
    static const std::array<int, 64> king_pst_mid;
    static const std::array<int, 64> king_pst_end;
    static const std::array<int, 64> pawn_pst_end;
};

#endif // AIPLAYER_H