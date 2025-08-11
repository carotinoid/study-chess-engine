#include "AIPlayer.h"
#include "Board.h"
#include "Piece.h"
#include <limits> // For std::numeric_limits
#include <vector> // For std::vector
#include <algorithm> // For std::max, std::min
#include <map> // For std::map


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

    // Piece values map
    std::map<PieceType, int> pieceValues = {
        {PieceType::PAWN, 100},
        {PieceType::KNIGHT, 300},
        {PieceType::BISHOP, 300},
        {PieceType::ROOK, 500},
        {PieceType::QUEEN, 900},
        {PieceType::KING, 0} // King value is handled by king safety
    };

    // 1. Material Evaluation
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            Square currentSquare = {i, j};
            const Piece* piece = board.getPieceAt(currentSquare);
            if (piece) {
                score += pieceValues[piece->getType()] * ((piece->getColor() == Color::WHITE) ? 1 : -1);
            }
        }
    }

    // Define evaluation constants
    const int MOBILITY_PAWN = 1;
    const int MOBILITY_KNIGHT = 3;
    const int MOBILITY_BISHOP = 3;
    const int MOBILITY_ROOK = 2;
    const int MOBILITY_QUEEN = 1;
    const int MOBILITY_KING = 1;

    const int KING_CASTLED_BONUS = 50;
    const int KING_PAWN_SHIELD_BONUS = 10;

    const int PAWN_ISOLATED_PENALTY = -20;
    const int PAWN_DOUBLED_PENALTY = -10;
    const int PAWN_PASSED_BONUS_BASE = 30;
    const int PAWN_PASSED_BONUS_RANK = 10;

    const int CENTER_CONTROL_BONUS = 10;

    // Iterate through all pieces for advanced evaluations
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Square currentSquare = {x, y};
            const Piece* piece = board.getPieceAt(currentSquare);
            if (!piece) continue;

            Color pieceColor = piece->getColor();
            int pieceValueMultiplier = (pieceColor == Color::WHITE) ? 1 : -1;

            // 2. Piece Mobility Evaluation
            std::vector<Move> legalMoves = piece->getPossibleMoves(board);
            int mobilityScore = 0;
            switch (piece->getType()) {
                case PieceType::PAWN: mobilityScore = legalMoves.size() * MOBILITY_PAWN; break;
                case PieceType::KNIGHT: mobilityScore = legalMoves.size() * MOBILITY_KNIGHT; break;
                case PieceType::BISHOP: mobilityScore = legalMoves.size() * MOBILITY_BISHOP; break;
                case PieceType::ROOK: mobilityScore = legalMoves.size() * MOBILITY_ROOK; break;
                case PieceType::QUEEN: mobilityScore = legalMoves.size() * MOBILITY_QUEEN; break;
                case PieceType::KING: mobilityScore = legalMoves.size() * MOBILITY_KING; break;
                default: break;
            }
            score += mobilityScore * pieceValueMultiplier;

            // 3. King Safety Evaluation
            if (piece->getType() == PieceType::KING) {
                // Castling bonus (simplified check: if king is on castled square)
                // This is a heuristic and might not be perfectly accurate without knowing game history
                if (pieceColor == Color::WHITE) {
                    if (piece->getPosition().file == 6 && piece->getPosition().rank == 0) { // White King side castled
                        score += KING_CASTLED_BONUS * pieceValueMultiplier;
                    } else if (piece->getPosition().file == 2 && piece->getPosition().rank == 0) { // White Queen side castled
                        score += KING_CASTLED_BONUS * pieceValueMultiplier;
                    }
                } else { // Black King
                    if (piece->getPosition().file == 6 && piece->getPosition().rank == 7) { // Black King side castled
                        score += KING_CASTLED_BONUS * pieceValueMultiplier;
                    } else if (piece->getPosition().file == 2 && piece->getPosition().rank == 7) { // Black Queen side castled
                        score += KING_CASTLED_BONUS * pieceValueMultiplier;
                    }
                }

                // Pawn shield (simplified: check pawns in front of king)
                // For white king at (x, y), check (x-1, y+1), (x, y+1), (x+1, y+1)
                // For black king at (x, y), check (x-1, y-1), (x, y-1), (x+1, y-1)
                int pawnShieldRank = (pieceColor == Color::WHITE) ? y + 1 : y - 1;
                for (int fileOffset = -1; fileOffset <= 1; ++fileOffset) {
                    int pawnShieldFile = x + fileOffset;
                    Square shieldSquare = {pawnShieldFile, pawnShieldRank};
                    if (shieldSquare.isValid()) {
                        const Piece* shieldPiece = board.getPieceAt(shieldSquare);
                        if (shieldPiece && shieldPiece->getType() == PieceType::PAWN && shieldPiece->getColor() == pieceColor) {
                            score += KING_PAWN_SHIELD_BONUS * pieceValueMultiplier;
                        }
                    }
                }
            }

            // 4. Pawn Structure Evaluation
            if (piece->getType() == PieceType::PAWN) {
                // Isolated Pawn: No friendly pawns on adjacent files
                bool isIsolated = true;
                for (int fileOffset = -1; fileOffset <= 1; ++fileOffset) {
                    if (fileOffset == 0) continue; // Skip current file
                    for (int rank = 0; rank < 8; ++rank) {
                        Square adjacentSquare = {x + fileOffset, rank};
                        if (adjacentSquare.isValid()) {
                            const Piece* adjacentPawn = board.getPieceAt(adjacentSquare);
                            if (adjacentPawn && adjacentPawn->getType() == PieceType::PAWN && adjacentPawn->getColor() == pieceColor) {
                                isIsolated = false;
                                break;
                            }
                        }
                    }
                    if (!isIsolated) break;
                }
                if (isIsolated) {
                    score += PAWN_ISOLATED_PENALTY * pieceValueMultiplier;
                }

                // Doubled Pawn: Another friendly pawn on the same file
                bool isDoubled = false;
                for (int rank = 0; rank < 8; ++rank) {
                    if (rank == y) continue; // Skip current rank
                    Square sameFileSquare = {x, rank};
                    const Piece* sameFilePawn = board.getPieceAt(sameFileSquare);
                    if (sameFilePawn && sameFilePawn->getType() == PieceType::PAWN && sameFilePawn->getColor() == pieceColor) {
                        isDoubled = true;
                        break;
                    }
                }
                if (isDoubled) {
                    score += PAWN_DOUBLED_PENALTY * pieceValueMultiplier;
                }

                // Passed Pawn: No opposing pawns on the same or adjacent files in front of it
                bool isPassed = true;
                int direction = (pieceColor == Color::WHITE) ? 1 : -1;
                for (int rank = y + direction; (pieceColor == Color::WHITE) ? (rank < 8) : (rank >= 0); rank += direction) {
                    for (int fileOffset = -1; fileOffset <= 1; ++fileOffset) {
                        int checkFile = x + fileOffset;
                        Square blockingSquare = {checkFile, rank};
                        if (blockingSquare.isValid()) {
                            const Piece* blockingPawn = board.getPieceAt(blockingSquare);
                            if (blockingPawn && blockingPawn->getType() == PieceType::PAWN && blockingPawn->getColor() != pieceColor) {
                                isPassed = false;
                                break;
                            }
                        }
                    }
                    if (!isPassed) break;
                }
                if (isPassed) {
                    int rankAdvanced = (pieceColor == Color::WHITE) ? (y - 1) : (6 - y); // Distance from starting rank (rank 1 for white, rank 6 for black)
                    score += (PAWN_PASSED_BONUS_BASE + rankAdvanced * PAWN_PASSED_BONUS_RANK) * pieceValueMultiplier;
                }
            }

            // 5. Center Control Evaluation
            // Center squares: (3,3) d5, (4,3) e5, (3,4) d4, (4,4) e4
            if ((x == 3 && y == 3) || (x == 4 && y == 3) || (x == 3 && y == 4) || (x == 4 && y == 4)) {
                score += CENTER_CONTROL_BONUS * pieceValueMultiplier;
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