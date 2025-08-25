#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "Bitboard.h"
#include "DataTypes.h"
#include <array>
#include <memory>

#include "TranspositionTable.h"
#include "Book.h"

class Game; // Forward Declaration

class AIPlayer {
public:
    AIPlayer(); // Constructor
    Move findBestMove(Game& game, int depth);
    void setUseOpeningBook(bool useBook);

    // --- Evaluation Weights ---
    // These are public so they can be accessed by helper functions.
    // They are const, so they cannot be changed.
    static const int PAWN_VALUE = 100;
    static const int KNIGHT_VALUE = 320;
    static const int BISHOP_VALUE = 330;
    static const int ROOK_VALUE = 500;
    static const int QUEEN_VALUE = 900;
    static const int KING_VALUE = 20000;

    static const int PST_WEIGHT = 1; 
    static const int MOBILITY_WEIGHT = 5;
    static const int KING_SAFETY_CASTLE_BONUS = 50;
    static const int KING_SAFETY_PAWN_SHIELD_BONUS = 10;
    static const int PAWN_DOUBLED_PENALTY = 20;
    static const int PAWN_ISOLATED_PENALTY = 15;
    static const int PAWN_PASSED_BONUS = 10;
    static const int CENTER_CONTROL_WEIGHT = 5;
    static const int ATTACKED_PIECE_PENALTY_FACTOR = 4;

    // Advanced Evaluation Weights
    static const int BISHOP_PAIR_BONUS = 50;
    static const int ROOK_ON_OPEN_FILE_BONUS = 50;
    static const int ROOK_ON_SEMI_OPEN_FILE_BONUS = 25;

    static const int MATE_SCORE = 32000;
    static const int MAX_PLY = 64;

private:
    std::unique_ptr<OpeningBook> openingBook;
    std::unique_ptr<TranspositionTable> transpositionTable;
    bool useOpeningBook = true;

    std::array<std::array<Move, 2>, MAX_PLY> killerMoves;
    std::array<std::array<int, 64>, 64> historyScores;

    void clearKillerMoves();
    void clearHistoryScores();

    int minimax(Game& game, int depth, int ply, bool maximizingPlayer, int alpha, int beta);
    int quiescenceSearch(Game& game, int alpha, int beta);
    int evaluate(const BoardState& boardState, const GameStatus& status);

    // --- Evaluation Components ---
    int calculateGamePhase(const BoardState& boardState) const;
    int evaluateMaterial(const BoardState& boardState) const;
    int evaluatePositional(const BoardState& boardState, int gamePhase) const;
    int evaluateMobility(const BoardState& boardState) const;
    int evaluateKingSafety(const BoardState& boardState) const;
    int evaluatePawnStructure(const BoardState& boardState) const;
    int evaluateStrategic(const BoardState& boardState) const; // Covers various strategic aspects

    // Piece-Square Tables (inline static for C++17)
    inline static const std::array<int, 64> pawn_pst = {
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };
    inline static const std::array<int, 64> knight_pst = {
        -50,-25,-15,-15,-15,-15,-25,-50,
        -25,-20,  0,  0,  0,  0,-20,-25,
        -15,  0, 10, 15, 15, 10,  0,-15,
        -15,  5, 15, 20, 20, 15,  5,-15,
        -15,  0, 15, 20, 20, 15,  0,-15,
        -15,  5, 10, 15, 15, 10,  5,-15,
        -25,-20,  0,  5,  5,  0,-20,-40,
        -50,-25,-15,-15,-15,-15,-25,-50,
    };
    inline static const std::array<int, 64> bishop_pst = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    };
    inline static const std::array<int, 64> rook_pst = {
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };
    inline static const std::array<int, 64> queen_pst = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };
    inline static const std::array<int, 64> king_pst_mid = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    };
    inline static const std::array<int, 64> king_pst_end = {
        -50,-40,-30,-20,-20,-30,-40,-50,
        -30,-20,-10,  0,  0,-10,-20,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 30, 40, 40, 30,-10,-30,
        -30,-10, 20, 30, 30, 20,-10,-30,
        -30,-30,  0,  0,  0,  0,-30,-30,
        -50,-30,-30,-30,-30,-30,-30,-50
    };
    inline static const std::array<int, 64> pawn_pst_end = {
        0,  0,  0,  0,  0,  0,  0,  0,
        80, 80, 80, 80, 80, 80, 80, 80,
        50, 50, 50, 50, 50, 50, 50, 50,
        30, 30, 30, 30, 30, 30, 30, 30,
        20, 20, 20, 20, 20, 20, 20, 20,
        10, 10, 10, 10, 10, 10, 10, 10,
        5,  5,  5,  5,  5,  5,  5,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };
};

#endif // AIPLAYER_H