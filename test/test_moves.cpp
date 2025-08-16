#include "test_helpers.h"
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <tuple>
#include "../include/Game.h"
#include "../include/MoveGen.h"
#include "../include/MagicBitboards.h"
#include "../include/Zobrist.h"
#include "../include/Debug.h"

#define SETUP(FEN) \
Game game(FEN); \
__last_game_for_debug = &game; \
const BoardState& state = game.getBoard().getState(); \
std::vector<Move> all_moves; \
MoveGen::generateMoves(state, all_moves); \


std::ostream& operator<<(std::ostream& os, const std::set<std::string>& moves) {
    os << "{ ";
    for (const auto& move : moves) {
        os << move << " ";
    }
    os << "}";
    return os;
}

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

bool compareMoves(
    const std::vector<Move>& calculated_moves, 
    const std::set<std::string>& expected_moves,
    std::string tooltip = "Moves comparison") {
    
    std::set<std::string> correct_moves, incorrect_moves, missing_moves;
    
    for (const auto& move : calculated_moves) {
        std::string move_str = move.toString();
        if (expected_moves.find(move_str) != expected_moves.end()) {
            correct_moves.insert(move_str);
        } else {
            incorrect_moves.insert(move_str);
        }
    }
    
    for (const auto& expected_move : expected_moves) {
        if (correct_moves.find(expected_move) == correct_moves.end()) {
            missing_moves.insert(expected_move);
        }
    }
    
    std::cout << "-- " << tooltip << std::endl;
    std::cout << "Correct Moves\t: " << correct_moves << std::endl;
    std::cout << "Incorrect Moves\t: " << incorrect_moves << std::endl;
    std::cout << "Missing Moves\t: " << missing_moves << std::endl;
    std::cout << std::endl;

    return correct_moves.size() == expected_moves.size() &&
           incorrect_moves.empty() && 
           missing_moves.empty();
}

bool exist(std::optional<std::pair<PieceType, Color>> piece, PieceType type, Color color) {
    return piece.has_value() && piece->first == type && piece->second == color;
}


// Forward declarations
void run_test_initial_position_move_count();
void run_test_knight_moves_from_initial_position();
void run_test_pawn_enpassant_moves();
void run_test_knight_move();
void run_test_bishop_move();
void run_test_rook_move();
void run_test_queen_moves();
void run_test_queen_moves_2();
void run_test_scholar_mate();

void run_move_tests() {
    Zobrist::Init();
    MagicBitboards::Init();
    std::cout << "\n--- Running MoveGen tests... ---" << std::endl;
    run_test_initial_position_move_count();
    run_test_knight_moves_from_initial_position();
    run_test_pawn_enpassant_moves();
    run_test_knight_move();
    run_test_bishop_move();
    run_test_rook_move();
    run_test_queen_moves();
    run_test_queen_moves_2();
    run_test_scholar_mate();
    std::cout << "--- MoveGen tests passed! ---" << std::endl;
}

TEST_CASE(test_initial_position_move_count) {
    SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    ASSERT_EQ(all_moves.size(), (size_t)20);
    // At the start of the game, there are 20 possible moves (16 pawn moves and 4 knight moves).
}

TEST_CASE(test_knight_moves_from_initial_position) {
    SETUP("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Square b1 = {0, 1};
    Square g1 = {0, 6};
    auto piece_b1 = getPieceAt(state, b1);
    ASSERT_TRUE(exist(piece_b1, PieceType::KNIGHT, Color::WHITE));
    std::vector<Move> knight_b1_moves = filterMovesByStartSquare(all_moves, b1);
    ASSERT_EQ(knight_b1_moves.size(), (size_t)2); // b1-a3, b1-c3
    auto piece_g1 = getPieceAt(state, g1);
    ASSERT_TRUE(exist(piece_g1, PieceType::KNIGHT, Color::WHITE));
    std::vector<Move> knight_g1_moves = filterMovesByStartSquare(all_moves, g1);
    ASSERT_EQ(knight_g1_moves.size(), (size_t)2); // g1-f3, g1-h3
}

TEST_CASE(test_pawn_enpassant_moves) {
    SETUP("rnbqkbnr/ppp1p2p/3p2p1/4Pp2/8/2P5/PP1P1PPP/RNBQKBNR w KQkq f6 0 4")
    Square e5 = {4, 4}; 
    Square f2 = {1, 5}; 
    Square c3 = {2, 2}; 
    auto piece_e5 = getPieceAt(state, e5);
    auto piece_f2 = getPieceAt(state, f2);
    auto piece_c3 = getPieceAt(state, c3);
    ASSERT_TRUE(exist(piece_e5, PieceType::PAWN, Color::WHITE));
    ASSERT_TRUE(exist(piece_f2, PieceType::PAWN, Color::WHITE));
    ASSERT_TRUE(exist(piece_c3, PieceType::PAWN, Color::WHITE));
    std::vector<Move> pawn_e5_moves = filterMovesByStartSquare(all_moves, e5);
    std::vector<Move> pawn_f2_moves = filterMovesByStartSquare(all_moves, f2);
    std::vector<Move> pawn_c3_moves = filterMovesByStartSquare(all_moves, c3);
    std::set<std::string> expected_e5_moves = {"e5f6", "e5d6", "e5e6"};
    std::set<std::string> expected_f2_moves = {"f2f3", "f2f4"};
    std::set<std::string> expected_c3_moves = {"c3c4"};
    ASSERT_TRUE(compareMoves(pawn_e5_moves, expected_e5_moves, "Pawn Moves from e5"));
    ASSERT_TRUE(compareMoves(pawn_f2_moves, expected_f2_moves, "Pawn Moves from f2"));
    ASSERT_TRUE(compareMoves(pawn_c3_moves, expected_c3_moves, "Pawn Moves from c3"));
}

TEST_CASE(test_knight_move) {
    SETUP("rnbqkbnr/pp1p1pp1/2p1p2p/8/3N4/2P5/PP1PPPPP/RNBQKB1R w KQkq - 0 4");
    Square d4 = {3, 3};
    auto piece_d4 = getPieceAt(state, d4);
    ASSERT_TRUE(exist(piece_d4, PieceType::KNIGHT, Color::WHITE));
    std::vector<Move> knight_d4_moves = filterMovesByStartSquare(all_moves, d4);
    std::set<std::string> expected_moves = {"d4c2", "d4b3", "d4b5", "d4f3", "d4f5", "d4c6", "d4e6"};
    ASSERT_TRUE(compareMoves(knight_d4_moves, expected_moves, "Knight Moves from d4"));
}

TEST_CASE(test_bishop_move) {
    SETUP("8/p1ppp2p/1p1k2r1/8/4B3/6P1/1N3PKP/8 w - - 0 1");
    Square e4 = {3, 4};
    auto piece_e4 = getPieceAt(state, e4);
    ASSERT_TRUE(exist(piece_e4, PieceType::BISHOP, Color::WHITE));
    std::vector<Move> bishop_e4_moves = filterMovesByStartSquare(all_moves, e4);
    std::set<std::string> expected_moves = {"e4d3", "e4c2", "e4b1", "e4f3", "e4f5", "e4g6", "e4d5", "e4c6", "e4b7", "e4a8"};
    ASSERT_TRUE(compareMoves(bishop_e4_moves, expected_moves, "Bishop Moves from e4"));
}

TEST_CASE(test_rook_move) {
    SETUP("3k4/2pp4/4p3/8/Q2R2n1/2P3P1/3P1PKP/8 w - - 0 1");
    Square d4 = {3, 3};
    auto piece_d4 = getPieceAt(state, d4);
    ASSERT_TRUE(exist(piece_d4, PieceType::ROOK, Color::WHITE));
    std::vector<Move> rook_d4_moves = filterMovesByStartSquare(all_moves, d4);
    std::set<std::string> expected_moves = {"d4d3", "d4c4", "d4b4", "d4e4", "d4f4", "d4g4", "d4d5", "d4d6", "d4d7"};
    ASSERT_TRUE(compareMoves(rook_d4_moves, expected_moves, "Rook Moves from d4"));
}

TEST_CASE(test_queen_moves) {
    SETUP("r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3");
    Square d1 = {0, 3};
    auto piece_d1 = getPieceAt(state, d1);
    ASSERT_TRUE(exist(piece_d1, PieceType::QUEEN, Color::WHITE));
    std::vector<Move> queen_d1_moves = filterMovesByStartSquare(all_moves, d1);
    std::set<std::string> expected_moves = {"d1d2", "d1d3", "d1d4", "d1e2", "d1f3", "d1g4", "d1h5"};
    ASSERT_TRUE(compareMoves(queen_d1_moves, expected_moves, "Queen Moves from d1"));
}
    

TEST_CASE(test_queen_moves_2) {
    SETUP("r1b1kb1r/ppppnppp/2n1pq2/8/4P3/1P3N2/P1PPBPPP/RNBQ1RK1 b kq - 0 5");
    Square f6 = {5, 5};
    auto piece_f6 = getPieceAt(state, f6);
    ASSERT_TRUE(exist(piece_f6, PieceType::QUEEN, Color::BLACK));
    std::vector<Move> queen_f6_moves = filterMovesByStartSquare(all_moves, f6);
    std::set<std::string> expected_moves = {"f6g6", "f6h6", "f6g5", "f6h4", "f6f5", "f6f4", "f6f3", 
        "f6e5", "f6d4", "f6c3", "f6b2", "f6a1"};
    ASSERT_TRUE(compareMoves(queen_f6_moves, expected_moves, "Queen Moves from f6"));
}

TEST_CASE(test_scholar_mate) {
    SETUP("r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4");
    Square c4 = {3, 2};
    Square f3 = {2, 5};
    auto piece_c4 = getPieceAt(state, c4);
    auto piece_f3 = getPieceAt(state, f3);
    ASSERT_TRUE(exist(piece_c4, PieceType::BISHOP, Color::WHITE));
    ASSERT_TRUE(exist(piece_f3, PieceType::QUEEN, Color::WHITE));
    std::vector<Move> bishop_c4_moves = filterMovesByStartSquare(all_moves, c4);
    std::vector<Move> queen_f3_moves = filterMovesByStartSquare(all_moves, f3);
    std::set<std::string> expected_bishop_moves = {"c4b3", "c4b5", "c4a6", "c4d3", "c4e2", "c4f1", "c4d5", "c4e6", "c4f7"},
        expected_queen_moves = {"f3a3", "f3b3", "f3c3", "f3d3", "f3e3", "f3g3", "f3h3", "f3e2", "f3d1", "f3g4", "f3h5", "f3f4", "f3f5", "f3f6", "f3f7"};
    ASSERT_TRUE(compareMoves(bishop_c4_moves, expected_bishop_moves, "Bishop Moves from c4"));
    ASSERT_TRUE(compareMoves(queen_f3_moves, expected_queen_moves, "Queen Moves from f3"));
}