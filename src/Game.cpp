#include "Game.h"
#include "Piece.h"
#include <cmath>

Game::Game() : status(GameStatus::ONGOING) {}

Board& Game::getBoard() { return board; }
const Board& Game::getBoard() const { return board; }
GameStatus Game::getStatus() const { return status; }

std::vector<Move> Game::getLegalMovesForPieceAt(Square s) {
    Piece* p = board.getPieceAt(s);
    if (!p || p->getColor() != board.getCurrentTurn()) return {};
    std::vector<Move> pseudoLegalMoves = p->getPossibleMoves(board);
    std::vector<Move> legalMoves;
    for (const auto& move : pseudoLegalMoves) {
        Board tempBoard = board;
        std::unique_ptr<Piece> movedPiece = tempBoard.takePieceAt(move.start);
        if (!movedPiece) continue;
        if (movedPiece->getType() == PieceType::PAWN && tempBoard.getEnPassantTarget().has_value() && move.end == *tempBoard.getEnPassantTarget() && tempBoard.getPieceAt(move.end) == nullptr) {
             int capturedPawnRank = (movedPiece->getColor() == Color::WHITE) ? move.end.rank - 1 : move.end.rank + 1;
             tempBoard.takePieceAt({capturedPawnRank, move.end.file});
        } else {
            tempBoard.takePieceAt(move.end);
        }
        tempBoard.placePieceAt(std::move(movedPiece), move.end);
        if (!tempBoard.isKingInCheck(p->getColor())) {
            legalMoves.push_back(move);
        }
    }
    return legalMoves;
}

std::vector<Move> Game::generateAllLegalMoves() {
    std::vector<Move> allLegalMoves;
    Color currentTurn = board.getCurrentTurn();
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            if (board.getPieceAt({r, f}) && board.getPieceAt({r, f})->getColor() == currentTurn) {
                std::vector<Move> pieceMoves = getLegalMovesForPieceAt({r, f});
                allLegalMoves.insert(allLegalMoves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }
    return allLegalMoves;
}

bool Game::makeMove(const Move& move) {
    Piece* pieceToMove = board.getPieceAt(move.start);
    if (!pieceToMove || pieceToMove->getColor() != board.getCurrentTurn()) return false;
    
    std::optional<Square> prevEnPassantTarget = board.getEnPassantTarget();
    board.setEnPassantTarget(std::nullopt);

    std::unique_ptr<Piece> movedPiece = board.takePieceAt(move.start);
    PieceType movedPieceType = movedPiece->getType();

    if (movedPieceType == PieceType::PAWN) {
        if (std::abs(move.end.rank - move.start.rank) == 2) {
            board.setEnPassantTarget(Square{(movedPiece->getColor() == Color::WHITE) ? move.start.rank + 1 : move.start.rank - 1, move.start.file});
        }
        if (prevEnPassantTarget.has_value() && move.end == *prevEnPassantTarget && !board.getPieceAt(move.end)) {
            board.takePieceAt({(movedPiece->getColor() == Color::WHITE) ? move.end.rank - 1 : move.end.rank + 1, move.end.file});
        }
    }
    if (movedPieceType == PieceType::KING && std::abs(move.end.file - move.start.file) == 2) {
        int rookFile = (move.end.file > move.start.file) ? 7 : 0;
        int newRookFile = (move.end.file > move.start.file) ? 5 : 3;
        std::unique_ptr<Piece> rook = board.takePieceAt({move.start.rank, rookFile});
        board.placePieceAt(std::move(rook), {move.start.rank, newRookFile});
    }

    board.takePieceAt(move.end);
    board.placePieceAt(std::move(movedPiece), move.end);

    if (move.promotionPiece.has_value()) {
        Color color = board.getPieceAt(move.end)->getColor();
        board.takePieceAt(move.end);
        std::unique_ptr<Piece> newPiece;
        switch(move.promotionPiece.value()) {
            case PieceType::QUEEN:  newPiece = std::make_unique<Queen>(color, move.end); break;
            case PieceType::ROOK:   newPiece = std::make_unique<Rook>(color, move.end); break;
            case PieceType::BISHOP: newPiece = std::make_unique<Bishop>(color, move.end); break;
            case PieceType::KNIGHT: newPiece = std::make_unique<Knight>(color, move.end); break;
            default: break;
        }
        board.placePieceAt(std::move(newPiece), move.end);
    }
    
    CastleRights rights = board.getCastleRights();
    if (movedPieceType == PieceType::KING) {
        if (pieceToMove->getColor() == Color::WHITE) { rights.whiteKingSide = rights.whiteQueenSide = false; } 
        else { rights.blackKingSide = rights.blackQueenSide = false; }
    }
    if (move.start == Square{0, 0} || move.end == Square{0, 0}) rights.whiteQueenSide = false;
    if (move.start == Square{0, 7} || move.end == Square{0, 7}) rights.whiteKingSide = false;
    if (move.start == Square{7, 0} || move.end == Square{7, 0}) rights.blackQueenSide = false;
    if (move.start == Square{7, 7} || move.end == Square{7, 7}) rights.blackKingSide = false;
    board.setCastleRights(rights);

    board.switchTurn();
    moveHistory.push_back(move);
    updateStatus();
    return true;
}

void Game::updateStatus() {
    std::vector<Move> legalMoves = generateAllLegalMoves();
    if (legalMoves.empty()) {
        status = board.isKingInCheck(board.getCurrentTurn()) ? GameStatus::CHECKMATE : GameStatus::STALEMATE;
    } else {
        status = GameStatus::ONGOING;
    }
}