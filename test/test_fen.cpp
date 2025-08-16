#include "test_helpers.h"
#include "../include/Game.h"
#include <iostream>

void run_fen_tests() {
    __last_game_for_debug = nullptr; // Reset last game for debug
    std::cout << "\n--- Running FEN Parsing Tests... ---" << std::endl;

    // Test Case 1: Kiwipete FEN
    std::string fen1 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    Game game1(fen1);
    __last_game_for_debug = &game1; // Set last game for debug
    const BoardState& state1 = game1.getBoard().getState();

    // Verify piece positions using ASSERT_TRUE with bitwise AND
    ASSERT_TRUE((state1.rook[1] & BitboardUtils::squareToBitboard({7, 0})) != 0); // a8
    ASSERT_TRUE((state1.king[1] & BitboardUtils::squareToBitboard({7, 4})) != 0); // e8
    ASSERT_TRUE((state1.rook[1] & BitboardUtils::squareToBitboard({7, 7})) != 0); // h8
    ASSERT_TRUE((state1.pawn[0] & BitboardUtils::squareToBitboard({3, 4})) != 0); // e4
    ASSERT_TRUE((state1.knight[0] & BitboardUtils::squareToBitboard({2, 2})) != 0); // c3
    ASSERT_TRUE((state1.queen[0] & BitboardUtils::squareToBitboard({2, 5})) != 0); // f3
    ASSERT_TRUE((state1.king[0] & BitboardUtils::squareToBitboard({0, 4})) != 0); // e1

    // Verify game state
    ASSERT_EQ(state1.currentTurn, Color::WHITE);
    ASSERT_EQ(state1.castleRights.whiteKingSide, true);
    ASSERT_EQ(state1.castleRights.whiteQueenSide, true);
    ASSERT_EQ(state1.castleRights.blackKingSide, true);
    ASSERT_EQ(state1.castleRights.blackQueenSide, true);
    ASSERT_EQ(state1.enPassantTarget.has_value(), false);
    ASSERT_EQ(state1.halfmoveClock, 0);
    ASSERT_EQ(state1.fullmoveNumber, 1);

    // Test Case 2: Position with en passant target
    std::string fen2 = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    Game game2(fen2);
    __last_game_for_debug = &game2; // Set last game for debug
    const BoardState& state2 = game2.getBoard().getState();
    ASSERT_EQ(state2.currentTurn, Color::WHITE);
    ASSERT_EQ(state2.enPassantTarget.has_value(), true);
    ASSERT_EQ(state2.enPassantTarget->rank, 5); // c6 rank is 5 (0-indexed)
    ASSERT_EQ(state2.enPassantTarget->file, 2); // c6 file is 2 (0-indexed)
    ASSERT_EQ(state2.fullmoveNumber, 2);

    std::cout << "--- FEN Parsing Tests Passed! ---" << std::endl;
}


