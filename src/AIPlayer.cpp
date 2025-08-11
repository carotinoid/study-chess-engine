#include "AIPlayer.h"
#include "Piece.h"
#include <limits>
#include <algorithm>

const std::array<int, 64> AIPlayer::pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};
const std::array<int, 64> AIPlayer::knight_pst = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};
const std::array<int, 64> AIPlayer::bishop_pst = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};
const std::array<int, 64> AIPlayer::rook_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};
const std::array<int, 64> AIPlayer::queen_pst = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};
const std::array<int, 64> AIPlayer::king_pst_mid = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

int AIPlayer::evaluate(const Board& board) {
    int score = 0;
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            const Piece* p = board.getPieceAt({r, f});
            if (p) {
                int pieceValue = 0;
                int pstValue = 0;
                int pos_idx = p->getColor() == Color::WHITE ? (r * 8 + f) : ((7 - r) * 8 + f);

                switch (p->getType()) {
                    case PieceType::PAWN:   pieceValue = 100; pstValue = pawn_pst[pos_idx]; break;
                    case PieceType::KNIGHT: pieceValue = 320; pstValue = knight_pst[pos_idx]; break;
                    case PieceType::BISHOP: pieceValue = 330; pstValue = bishop_pst[pos_idx]; break;
                    case PieceType::ROOK:   pieceValue = 500; pstValue = rook_pst[pos_idx]; break;
                    case PieceType::QUEEN:  pieceValue = 900; pstValue = queen_pst[pos_idx]; break;
                    case PieceType::KING:   pieceValue = 20000; pstValue = king_pst_mid[pos_idx]; break;
                }
                score += (p->getColor() == Color::WHITE) ? (pieceValue + pstValue) : -(pieceValue + pstValue);
            }
        }
    }
    return score;
}

int AIPlayer::minimax(Game& game, int depth, bool maximizingPlayer, int alpha, int beta) {
    if (depth == 0 || game.getStatus() != GameStatus::ONGOING) {
        return evaluate(game.getBoard());
    }

    std::vector<Move> legalMoves = game.generateAllLegalMoves();

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : legalMoves) {
            Game tempGame = game;
            tempGame.makeMove(move);
            int eval = minimax(tempGame, depth - 1, false, alpha, beta);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : legalMoves) {
            Game tempGame = game;
            tempGame.makeMove(move);
            int eval = minimax(tempGame, depth - 1, true, alpha, beta);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

Move AIPlayer::findBestMove(Game& game, int depth) {
    std::vector<Move> legalMoves = game.generateAllLegalMoves();
    Move bestMove;
    int bestValue = (game.getBoard().getCurrentTurn() == Color::WHITE) ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const auto& move : legalMoves) {
        Game tempGame = game;
        tempGame.makeMove(move);
        
        bool isMaximizing = (tempGame.getBoard().getCurrentTurn() == Color::WHITE);
        int boardValue = minimax(tempGame, depth - 1, isMaximizing, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

        if (game.getBoard().getCurrentTurn() == Color::WHITE) {
            if (boardValue > bestValue) {
                bestValue = boardValue;
                bestMove = move;
            }
        } else {
            if (boardValue < bestValue) {
                bestValue = boardValue;
                bestMove = move;
            }
        }
    }
    return bestMove;
}