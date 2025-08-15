#include "../include/Game.h"
#include "../include/MoveGen.h"

Game::Game() : status(GameStatus::ONGOING) {
    board.setupInitialPosition();
    updateStatus(); // Explicitly update status after setting up the board
}

Game::Game(const std::string& fen) : status(GameStatus::ONGOING) {
    board.setupPositionFromFen(fen);
    updateStatus();
}

BitboardRepresentation& Game::getBoard() { return board; }
const BitboardRepresentation& Game::getBoard() const { return board; }
GameStatus Game::getStatus() const { return status; }

std::vector<Move> Game::getLegalMovesForPieceAt(Square s) const {
    std::vector<Move> allLegalMoves = generateAllLegalMoves();
    std::vector<Move> pieceMoves;
    for (const auto& move : allLegalMoves) {
        if (move.start == s) {
            pieceMoves.push_back(move);
        }
    }
    return pieceMoves;
}

#include <iostream>

std::vector<Move> Game::generateAllLegalMoves() const {
    std::vector<Move> pseudoLegalMoves;
    MoveGen::generateMoves(board.getState(), pseudoLegalMoves);
    std::vector<Move> legalMoves;
    Color currentTurn = board.getState().currentTurn;

    for (const auto& move : pseudoLegalMoves) {
        BitboardRepresentation tempBoard = board;
        tempBoard.makeMove(move);
        bool isCheck = tempBoard.isKingInCheck(currentTurn);
        // std::cout << "Move: " << move.toString() << ", isCheck: " << isCheck << std::endl;
        if (!isCheck) {
            legalMoves.push_back(move);
        }
    }
    return legalMoves;
}

bool Game::makeMove(const Move& move) {
    // It is assumed that the move is legal.
    board.makeMove(move);
    moveHistory.push_back(move);
    updateStatus();
    return true;
}

void Game::updateStatus() {
    std::vector<Move> legalMoves = generateAllLegalMoves();

    if (legalMoves.empty()) {
        if (board.isKingInCheck(board.getState().currentTurn)) {
            status = GameStatus::CHECKMATE;
        } else {
            status = GameStatus::STALEMATE;
        }
    } else {
        status = GameStatus::ONGOING;
        // Check for insufficient material
        int white_pieces_count = __builtin_popcountll(board.getState().white_pieces);
        int black_pieces_count = __builtin_popcountll(board.getState().black_pieces);

        if (white_pieces_count == 1 && black_pieces_count == 1) { // K vs K
            status = GameStatus::DRAW;
        } else if (white_pieces_count == 1 && __builtin_popcountll(board.getState().knight[1]) == 1 && black_pieces_count == 2) { // K vs KN
            status = GameStatus::DRAW;
        } else if (white_pieces_count == 1 && __builtin_popcountll(board.getState().bishop[1]) == 1 && black_pieces_count == 2) { // K vs KB
            status = GameStatus::DRAW;
        } else if (black_pieces_count == 1 && __builtin_popcountll(board.getState().knight[0]) == 1 && white_pieces_count == 2) { // KN vs K
            status = GameStatus::DRAW;
        } else if (black_pieces_count == 1 && __builtin_popcountll(board.getState().bishop[0]) == 1 && white_pieces_count == 2) { // KB vs K
            status = GameStatus::DRAW;
        }
        // Fifty-move rule
        if (board.getState().halfmoveClock >= 100) {
            status = GameStatus::DRAW;
        }
    }
}
