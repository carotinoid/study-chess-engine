#include "test_helpers.h"
#include <iostream>
#include "../include/Game.h"
#include "../include/Bitboard.h"
#include "../include/Debug.h"

// Forward declarations
void run_test_make_move();
void run_test_render_board();

void run_board_tests() {
    std::cout << "\n--- Running Board tests... ---" << std::endl;
    run_test_make_move();
    run_test_render_board();
    std::cout << "--- Board tests passed! ---" << std::endl;
}

TEST_CASE(test_make_move) {
    Game game;
    // e2e4
    Move m = {Square{1, 4}, Square{3, 4}};
    game.makeMove(m);
    const BoardState& state = game.getBoard().getState();
    
    // Check if the pawn has moved from e2 to e4
    Bitboard e2_mask = 1ULL << (1 * 8 + 4);
    Bitboard e4_mask = 1ULL << (3 * 8 + 4);

    ASSERT_TRUE((state.pawn[0] & e4_mask) != 0); // Pawn should be on e4
    ASSERT_TRUE((state.pawn[0] & e2_mask) == 0); // Pawn should not be on e2
    ASSERT_EQ(state.currentTurn, Color::BLACK); // Turn should have switched to black
}

TEST_CASE(test_render_board) {
    Game game;
    std::cout << "\n# Rendering Initial Board" << std::endl;
    Debug::renderBoard(game);

    // Make a move and render again
    Move m = {Square{1, 4}, Square{3, 4}}; // e2e4
    game.makeMove(m);
    std::cout << "# Rendering Board after e2e4" << std::endl;
    Debug::renderBoard(game);
}
