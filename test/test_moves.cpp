#include "test_helpers.h"
#include <vector>
#include <iostream>
#include <optional>
#include "../include/Game.h"
#include "../include/MoveGen.h"
#include "../include/MagicBitboards.h"
#include "../include/Zobrist.h"

// Helper function to get the piece at a specific square
std::optional<std::pair<PieceType, Color>> getPieceAt(const BoardState& boardState, Square s) {
    Bitboard mask = 1ULL << (s.rank * 8 + s.file);
    for (int color_idx = 0; color_idx < 2; ++color_idx) {
        Color color = (color_idx == 0) ? Color::WHITE : Color::BLACK;
        if (boardState.pawn[color_idx] & mask) return {{PieceType::PAWN, color}};
        if (boardState.knight[color_idx] & mask) return {{PieceType::KNIGHT, color}};
        if (boardState.bishop[color_idx] & mask) return {{PieceType::BISHOP, color}};
        if (boardState.rook[color_idx] & mask) return {{PieceType::ROOK, color}};
        if (boardState.queen[color_idx] & mask) return {{PieceType::QUEEN, color}};
        if (boardState.king[color_idx] & mask) return {{PieceType::KING, color}};
    }
    return std::nullopt;
}

// Helper function to filter moves by starting square
std::vector<Move> filterMovesByStartSquare(const std::vector<Move>& moves, Square start) {
    std::vector<Move> filteredMoves;
    for (const auto& move : moves) {
        if (move.start == start) {
            filteredMoves.push_back(move);
        }
    }
    return filteredMoves;
}

// Forward declarations
void run_test_initial_position_move_count();
void run_test_knight_moves_from_initial_position();

void run_move_tests() {
    Zobrist::Init();
    MagicBitboards::Init();
    std::cout << "Running MoveGen tests..." << std::endl;
    run_test_initial_position_move_count();
    run_test_knight_moves_from_initial_position();
    std::cout << "MoveGen tests passed!" << std::endl;
}

TEST_CASE(test_initial_position_move_count) {
    Game game;
    std::vector<Move> moves;
    MoveGen::generateMoves(game.getBoard().getState(), moves);
    // At the start of the game, there are 20 possible moves (16 pawn moves and 4 knight moves).
    ASSERT_EQ(moves.size(), (size_t)20);
}

TEST_CASE(test_knight_moves_from_initial_position) {
    Game game;
    const BoardState& state = game.getBoard().getState();
    std::vector<Move> all_moves;
    MoveGen::generateMoves(state, all_moves);

    // Test white knights
    Square b1 = {0, 1};
    Square g1 = {0, 6};
    auto piece_b1 = getPieceAt(state, b1);
    ASSERT_TRUE(piece_b1.has_value() && piece_b1->first == PieceType::KNIGHT && piece_b1->second == Color::WHITE);
    std::vector<Move> knight_b1_moves = filterMovesByStartSquare(all_moves, b1);
    ASSERT_EQ(knight_b1_moves.size(), (size_t)2); // b1-a3, b1-c3

    auto piece_g1 = getPieceAt(state, g1);
    ASSERT_TRUE(piece_g1.has_value() && piece_g1->first == PieceType::KNIGHT && piece_g1->second == Color::WHITE);
    std::vector<Move> knight_g1_moves = filterMovesByStartSquare(all_moves, g1);
    ASSERT_EQ(knight_g1_moves.size(), (size_t)2); // g1-f3, g1-h3
}
