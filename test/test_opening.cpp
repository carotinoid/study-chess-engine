#include "test_helpers.h"
#include "../include/Game.h"
#include "../include/Bitboard.h"
#include <iostream>
#include <vector>
#include <iomanip>

// Forward declarations
void run_OpeningZobristHashGeneration();

void run_opening_tests() {
    std::cout << "\n--- Running Opening tests... ---" << std::endl;
    run_OpeningZobristHashGeneration();
    std::cout << "--- Opening tests passed! ---" << std::endl;
}

void printBoardStateForHashing(const BoardState& state) {
    std::cerr << "Turn: " << (state.currentTurn == Color::WHITE ? "White" : "Black") << std::endl;
    std::cerr << "Castling: " 
              << (state.castleRights.whiteKingSide ? "K" : "") 
              << (state.castleRights.whiteQueenSide ? "Q" : "")
              << (state.castleRights.blackKingSide ? "k" : "")
              << (state.castleRights.blackQueenSide ? "q" : "")
              << std::endl;
    std::cerr << "En Passant: " << (state.enPassantTarget.has_value() ? state.enPassantTarget.value().toString() : "-") << std::endl;
    std::cerr << "Zobrist Key: " << state.zobristKey << std::endl;
    std::cerr << "-------------------------" << std::endl;
}

Move findMove(Game& game, const std::string& moveStr) {
    std::vector<Move> moves = game.generateAllLegalMoves();
    for (const auto& m : moves) {
        if (m.toString() == moveStr) {
            return m;
        }
    }
    return Move{{-1,-1},{-1,-1}};
}

TEST_CASE(OpeningZobristHashGeneration) {
    Game game;
    std::cerr << "\n--- Initial Position ---" << std::endl;
    printBoardStateForHashing(game.getBoard().getState());

    // 1. e2e4
    Move move1 = findMove(game, "e2e4");
    ASSERT_TRUE(move1.start.isValid());
    game.makeMove(move1);
    std::cerr << "\n--- After e2e4 ---" << std::endl;
    printBoardStateForHashing(game.getBoard().getState());

    // 2. b8c6
    Move move2 = findMove(game, "b8c6");
    ASSERT_TRUE(move2.start.isValid());
    game.makeMove(move2);
    std::cerr << "\n--- After b8c6 ---" << std::endl;
    printBoardStateForHashing(game.getBoard().getState());

    // 3. g1f3
    Move move3 = findMove(game, "g1f3");
    ASSERT_TRUE(move3.start.isValid());
    game.makeMove(move3);
    std::cerr << "\n--- After g1f3 ---" << std::endl;
    printBoardStateForHashing(game.getBoard().getState());
}
