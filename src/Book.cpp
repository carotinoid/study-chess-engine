#include "Book.h"
#include "Game.h"
#include "DataTypes.h"
#include "Bitboard.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <optional>
#include <cctype>
#include <algorithm>

namespace {
    std::optional<PieceType> getPieceType(const BitboardRepresentation& board, Square square) {
        if (!square.isValid()) return std::nullopt;
        uint64_t mask = 1ULL << (square.rank * 8 + square.file);
        const auto& state = board.getState();
        if (state.pawn[0] & mask || state.pawn[1] & mask) return PieceType::PAWN;
        if (state.knight[0] & mask || state.knight[1] & mask) return PieceType::KNIGHT;
        if (state.bishop[0] & mask || state.bishop[1] & mask) return PieceType::BISHOP;
        if (state.rook[0] & mask || state.rook[1] & mask) return PieceType::ROOK;
        if (state.queen[0] & mask || state.queen[1] & mask) return PieceType::QUEEN;
        if (state.king[0] & mask || state.king[1] & mask) return PieceType::KING;
        return std::nullopt;
    }

    Square squareFromString(const std::string& str) {
        if (str.length() < 2) return {-1, -1};
        int file = str[0] - 'a';
        int rank = str[1] - '1';
        return {rank, file};
    }

    std::optional<PieceType> pieceTypeFromChar(char c) {
        switch (toupper(c)) {
            case 'N': return PieceType::KNIGHT;
            case 'B': return PieceType::BISHOP;
            case 'R': return PieceType::ROOK;
            case 'Q': return PieceType::QUEEN;
            case 'K': return PieceType::KING;
            case 'P': return PieceType::PAWN;
            default: return std::nullopt;
        }
    }
}

OpeningBook::OpeningBook(const std::string& ecoFile) : bookFile(ecoFile) {}

void OpeningBook::load() {
    std::ifstream file(bookFile);
    if (!file.is_open()) {
        std::cerr << "Could not open opening book file: " << bookFile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string eco, moves_str;
        ss >> eco;
        
        size_t first_quote_pos = line.find('"');
        size_t eco_end_pos = line.find(eco) + eco.length();
        if (first_quote_pos != std::string::npos) {
            moves_str = line.substr(eco_end_pos, first_quote_pos - eco_end_pos);
        } else {
            moves_str = line.substr(eco_end_pos);
        }

        moves_str.erase(0, moves_str.find_first_not_of(" 	"));
        moves_str.erase(moves_str.find_last_not_of(" 	") + 1);

        Game game;
        std::stringstream move_stream(moves_str);
        std::string move_str;

        while (move_stream >> move_str) {
            // If the string is a move number (e.g., "1."), skip it.
            if (move_str.find('.') != std::string::npos) {
                continue;
            }
            uint64_t current_hash = game.getBoard().getState().zobristKey;
            Move move = sanToMove(move_str, game);
            if (!move.start.isValid()) { // Check for null move
                break; 
            }
            book.insert({current_hash, move});
            game.makeMove(move);
        }
    }
    std::cout << "info string Opening book loaded with " << book.size() << " positions." << std::endl;
}

std::vector<Move> OpeningBook::findMoves(const uint64_t hash) const {
    auto range = book.equal_range(hash);
    std::vector<Move> moves;
    for (auto it = range.first; it != range.second; ++it) {
        moves.push_back(it->second);
    }
    return moves;
}

std::string moveToSan(const Move& move, const Game& game) {
    // This is a simplified SAN conversion, doesn't handle ambiguities
    const auto& board = game.getBoard();
    auto pieceTypeOpt = getPieceType(board, move.start);
    if (!pieceTypeOpt) return ""; // Should not happen

    PieceType pieceType = *pieceTypeOpt;
    std::string san;

    if (pieceType == PieceType::KING && abs(move.start.file - move.end.file) == 2) {
        if (move.end.file == 6) return "O-O";
        if (move.end.file == 2) return "O-O-O";
    }

    if (pieceType != PieceType::PAWN) {
        san += to_string(pieceType);
    }

    // Check for captures
    if (board.getState().all_pieces & BitboardUtils::squareToBitboard(move.end)) {
        if (pieceType == PieceType::PAWN) {
            san += (char)('a' + move.start.file);
        }
        san += 'x';
    }

    san += (char)('a' + move.end.file);
    san += (char)('1' + move.end.rank);

    if (move.promotionPiece) {
        san += '=';
        san += to_string(*move.promotionPiece);
    }

    // Create a temporary game to check for check/checkmate
    Game tempGame = game;
    tempGame.makeMove(move);
    if (tempGame.getStatus() == GameStatus::CHECKMATE) {
        san += '#';
    } else if (tempGame.getBoard().isKingInCheck(tempGame.getBoard().getState().currentTurn)) {
        san += '+';
    }

    return san;
}

Move OpeningBook::sanToMove(const std::string& san, Game& game) {
    std::vector<Move> legalMoves = game.generateAllLegalMoves();
    for (const auto& move : legalMoves) {
        if (moveToSan(move, game) == san) {
            return move;
        }
    }
    // A simplified fallback for pawn moves, as SAN for pawns is just the destination square
    if (san.length() == 2 && san[0] >= 'a' && san[0] <= 'h' && san[1] >= '1' && san[1] <= '8') {
        for (const auto& move : legalMoves) {
            auto pieceTypeOpt = getPieceType(game.getBoard(), move.start);
            if (pieceTypeOpt && *pieceTypeOpt == PieceType::PAWN) {
                std::string dest_san;
                dest_san += (char)('a' + move.end.file);
                dest_san += (char)('1' + move.end.rank);
                if (dest_san == san) {
                    return move;
                }
            }
        }
    }


    return Move{{-1,-1},{-1,-1}};
}