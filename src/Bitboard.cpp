#include "../include/Bitboard.h"
#include "../include/MoveGen.h"
#include "../include/MagicBitboards.h"
#include "../include/Zobrist.h"
#include <iostream>
#include <sstream>
#include <cctype>

namespace {
    // Constants for file masks, to prevent wrap-around when generating moves
    constexpr Bitboard not_a_file = 0xfefefefefefefefeULL; // ~A1
    constexpr Bitboard not_h_file = 0x7f7f7f7f7f7f7f7fULL; // ~H1
    constexpr Bitboard not_ab_file = 0xfcfcfcfcfcfcfcfcULL; // ~A1 & ~B1
    constexpr Bitboard not_gh_file = 0x3f3f3f3f3f3f3f3fULL; // ~G1 & ~H1
}

BitboardRepresentation::BitboardRepresentation() {
    setupInitialPosition();
}

void BitboardRepresentation::setupInitialPosition() {
    // White pieces
    currentState.pawn[0]   = 0x000000000000FF00ULL;
    currentState.knight[0] = 0x0000000000000042ULL;
    currentState.bishop[0] = 0x0000000000000024ULL;
    currentState.rook[0]   = 0x0000000000000081ULL;
    currentState.queen[0]  = 0x0000000000000008ULL;
    currentState.king[0]   = 0x0000000000000010ULL;

    // Black pieces
    currentState.pawn[1]   = 0x00FF000000000000ULL;
    currentState.knight[1] = 0x4200000000000000ULL;
    currentState.bishop[1] = 0x2400000000000000ULL;
    currentState.rook[1]   = 0x8100000000000000ULL;
    currentState.queen[1]  = 0x0800000000000000ULL;
    currentState.king[1]   = 0x1000000000000000ULL;

    updateCompositeBitboards();

    currentState.currentTurn = Color::WHITE;
    currentState.castleRights = {true, true, true, true};
    currentState.enPassantTarget = std::nullopt;
    currentState.halfmoveClock = 0;
    currentState.fullmoveNumber = 1;

    currentState.zobristKey = Zobrist::ComputeHash(currentState);
}

void BitboardRepresentation::setupPositionFromFen(const std::string& fen) {
    // Clear the board first
    for (int i = 0; i < 2; ++i) {
        currentState.pawn[i] = 0;
        currentState.knight[i] = 0;
        currentState.bishop[i] = 0;
        currentState.rook[i] = 0;
        currentState.queen[i] = 0;
        currentState.king[i] = 0;
    }
    currentState.enPassantTarget = std::nullopt;
    currentState.castleRights = {false, false, false, false};
    currentState.halfmoveClock = 0;
    currentState.fullmoveNumber = 1;

    std::istringstream iss(fen);
    std::string board_part;
    iss >> board_part;

    int rank = 7, file = 0;
    for (char c : board_part) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += c - '0';
        } else {
            Color color = isupper(c) ? Color::WHITE : Color::BLACK;
            int color_idx = (color == Color::WHITE) ? 0 : 1;
            Bitboard bb = 1ULL << (rank * 8 + file);
            switch (tolower(c)) {
                case 'p': currentState.pawn[color_idx] |= bb; break;
                case 'n': currentState.knight[color_idx] |= bb; break;
                case 'b': currentState.bishop[color_idx] |= bb; break;
                case 'r': currentState.rook[color_idx] |= bb; break;
                case 'q': currentState.queen[color_idx] |= bb; break;
                case 'k': currentState.king[color_idx] |= bb; break;
            }
            file++;
        }
    }

    std::string turn_part;
    iss >> turn_part;
    currentState.currentTurn = (turn_part == "w") ? Color::WHITE : Color::BLACK;

    std::string castling_part;
    iss >> castling_part;
    if (castling_part != "-") {
        for (char c : castling_part) {
            switch (c) {
                case 'K': currentState.castleRights.whiteKingSide = true; break;
                case 'Q': currentState.castleRights.whiteQueenSide = true; break;
                case 'k': currentState.castleRights.blackKingSide = true; break;
                case 'q': currentState.castleRights.blackQueenSide = true; break;
            }
        }
    }

    std::string enpassant_part;
    iss >> enpassant_part;
    if (enpassant_part != "-") {
        int ep_file = enpassant_part[0] - 'a';
        int ep_rank = enpassant_part[1] - '1';
        currentState.enPassantTarget = Square{ep_rank, ep_file};
    }

    iss >> currentState.halfmoveClock;
    iss >> currentState.fullmoveNumber;

    updateCompositeBitboards();
    currentState.zobristKey = Zobrist::ComputeHash(currentState);
}

const BoardState& BitboardRepresentation::getState() const {
    return currentState;
}

void BitboardRepresentation::updateCompositeBitboards() {
    currentState.white_pieces = currentState.pawn[0] | currentState.knight[0] | currentState.bishop[0] | currentState.rook[0] | currentState.queen[0] | currentState.king[0];
    currentState.black_pieces = currentState.pawn[1] | currentState.knight[1] | currentState.bishop[1] | currentState.rook[1] | currentState.queen[1] | currentState.king[1];
    currentState.all_pieces = currentState.white_pieces | currentState.black_pieces;
}

bool BitboardRepresentation::isKingInCheck(Color kingColor) const {
    int king_idx = (kingColor == Color::WHITE) ? 0 : 1;
    Bitboard king_bb = currentState.king[king_idx];
    if (king_bb == 0) return false; // No king on the board

    Square king_sq = (Square){(int)__builtin_ctzll(king_bb) / 8, (int)__builtin_ctzll(king_bb) % 8};
    return BitboardUtils::isSquareAttackedBy(king_sq, (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE, currentState);
}

void BitboardRepresentation::makeMove(const Move& move) {
    // --- Store old state for Zobrist update ---
    const int us_idx = (currentState.currentTurn == Color::WHITE) ? 0 : 1;
    const int them_idx = 1 - us_idx;
    const int oldCastleRights = (currentState.castleRights.whiteKingSide << 3) |
                              (currentState.castleRights.whiteQueenSide << 2) |
                              (currentState.castleRights.blackKingSide << 1) |
                              (currentState.castleRights.blackQueenSide);
    const std::optional<Square> oldEnPassantTarget = currentState.enPassantTarget;
    uint64_t hash = currentState.zobristKey;

    // --- Zobrist Update Part 1: Remove old state ---
    hash ^= Zobrist::blackToMoveKey; // Always switch side to move
    hash ^= Zobrist::castleKeys[oldCastleRights];
    if (oldEnPassantTarget) {
        hash ^= Zobrist::enPassantKeys[oldEnPassantTarget->file];
    }

    // --- Find Moving Piece ---
    Bitboard from_bb = BitboardUtils::squareToBitboard(move.start);
    Bitboard to_bb = BitboardUtils::squareToBitboard(move.end);
    Bitboard from_to_bb = from_bb | to_bb;
    PieceType moving_piece_type = PieceType::PAWN; // Default
    if (from_bb & currentState.pawn[us_idx]) moving_piece_type = PieceType::PAWN;
    else if (from_bb & currentState.knight[us_idx]) moving_piece_type = PieceType::KNIGHT;
    else if (from_bb & currentState.bishop[us_idx]) moving_piece_type = PieceType::BISHOP;
    else if (from_bb & currentState.rook[us_idx]) moving_piece_type = PieceType::ROOK;
    else if (from_bb & currentState.queen[us_idx]) moving_piece_type = PieceType::QUEEN;
    else if (from_bb & currentState.king[us_idx]) moving_piece_type = PieceType::KING;

    // --- Zobrist Update Part 2: Move piece ---
    int moving_piece_idx = us_idx * 6 + static_cast<int>(moving_piece_type);
    hash ^= Zobrist::pieceKeys[moving_piece_idx][move.start.rank * 8 + move.start.file]; // XOR out from old square
    hash ^= Zobrist::pieceKeys[moving_piece_idx][move.end.rank * 8 + move.end.file];   // XOR in to new square

    // --- Update Piece Bitboards ---
    auto move_piece_bb = [&](Bitboard& bb) { bb ^= from_to_bb; };
    switch(moving_piece_type) {
        case PieceType::PAWN:   move_piece_bb(currentState.pawn[us_idx]); break;
        case PieceType::KNIGHT: move_piece_bb(currentState.knight[us_idx]); break;
        case PieceType::BISHOP: move_piece_bb(currentState.bishop[us_idx]); break;
        case PieceType::ROOK:   move_piece_bb(currentState.rook[us_idx]); break;
        case PieceType::QUEEN:  move_piece_bb(currentState.queen[us_idx]); break;
        case PieceType::KING:   move_piece_bb(currentState.king[us_idx]); break;
    }

    // --- Handle Captures ---
    if (to_bb & currentState.all_pieces) {
        PieceType captured_piece_type = PieceType::PAWN;
        if (to_bb & currentState.pawn[them_idx]) { captured_piece_type = PieceType::PAWN; currentState.pawn[them_idx] ^= to_bb; }
        else if (to_bb & currentState.knight[them_idx]) { captured_piece_type = PieceType::KNIGHT; currentState.knight[them_idx] ^= to_bb; }
        else if (to_bb & currentState.bishop[them_idx]) { captured_piece_type = PieceType::BISHOP; currentState.bishop[them_idx] ^= to_bb; }
        else if (to_bb & currentState.rook[them_idx]) { captured_piece_type = PieceType::ROOK; currentState.rook[them_idx] ^= to_bb; }
        else if (to_bb & currentState.queen[them_idx]) { captured_piece_type = PieceType::QUEEN; currentState.queen[them_idx] ^= to_bb; }
        
        // --- Zobrist Update Part 3: Remove captured piece ---
        int captured_piece_idx = them_idx * 6 + static_cast<int>(captured_piece_type);
        hash ^= Zobrist::pieceKeys[captured_piece_idx][move.end.rank * 8 + move.end.file];
    }

    // --- Handle Special Moves ---
    // En Passant
    if (moving_piece_type == PieceType::PAWN && move.end == oldEnPassantTarget) {
        Bitboard captured_pawn_bb = (currentState.currentTurn == Color::WHITE) ? (to_bb >> 8) : (to_bb << 8);
        int captured_sq_idx = __builtin_ctzll(captured_pawn_bb);
        currentState.pawn[them_idx] ^= captured_pawn_bb;
        // --- Zobrist Update Part 3b: Remove en passant captured pawn ---
        hash ^= Zobrist::pieceKeys[them_idx * 6 + static_cast<int>(PieceType::PAWN)][captured_sq_idx];
    }

    // Promotion
    if (move.promotionPiece.has_value()) {
        currentState.pawn[us_idx] ^= to_bb; // Remove the pawn from the promotion square
        switch(*move.promotionPiece) { // Add the new piece
            case PieceType::QUEEN:  currentState.queen[us_idx]  |= to_bb; break;
            case PieceType::ROOK:   currentState.rook[us_idx]   |= to_bb; break;
            case PieceType::BISHOP: currentState.bishop[us_idx] |= to_bb; break;
            case PieceType::KNIGHT: currentState.knight[us_idx] |= to_bb; break;
            default: break;
        }
        // --- Zobrist Update Part 4: Handle promotion ---
        hash ^= Zobrist::pieceKeys[us_idx * 6 + static_cast<int>(PieceType::PAWN)][move.end.rank * 8 + move.end.file]; // remove pawn
        hash ^= Zobrist::pieceKeys[us_idx * 6 + static_cast<int>(*move.promotionPiece)][move.end.rank * 8 + move.end.file]; // add promoted piece
    }

    // Castling
    if (moving_piece_type == PieceType::KING && abs(move.start.file - move.end.file) == 2) {
        
        int rook_start_sq, rook_end_sq;
        if (move.end.file == 6) { // Kingside
            rook_start_sq = move.start.rank * 8 + 7;
            rook_end_sq = move.start.rank * 8 + 5;
        } else { // Queenside
            rook_start_sq = move.start.rank * 8 + 0;
            rook_end_sq = move.start.rank * 8 + 3;
        }
        currentState.rook[us_idx] ^= (BitboardUtils::squareToBitboard({rook_start_sq/8, rook_start_sq%8}) | BitboardUtils::squareToBitboard({rook_end_sq/8, rook_end_sq%8}));
        // --- Zobrist Update Part 5: Handle castling rook move ---
        hash ^= Zobrist::pieceKeys[us_idx * 6 + static_cast<int>(PieceType::ROOK)][rook_start_sq];
        hash ^= Zobrist::pieceKeys[us_idx * 6 + static_cast<int>(PieceType::ROOK)][rook_end_sq];
    }

    // --- Update Game State ---
    // Update castling rights
    if (moving_piece_type == PieceType::KING) {
        if (us_idx == 0) { currentState.castleRights.whiteKingSide = false; currentState.castleRights.whiteQueenSide = false; }
        else { currentState.castleRights.blackKingSide = false; currentState.castleRights.blackQueenSide = false; }
    }
    if (from_bb & 0x1) currentState.castleRights.whiteQueenSide = false;
    if (from_bb & 0x80) currentState.castleRights.whiteKingSide = false;
    if (from_bb & 0x0100000000000000) currentState.castleRights.blackQueenSide = false;
    if (from_bb & 0x8000000000000000) currentState.castleRights.blackKingSide = false;
    if (to_bb & 0x1) currentState.castleRights.whiteQueenSide = false;
    if (to_bb & 0x80) currentState.castleRights.whiteKingSide = false;
    if (to_bb & 0x0100000000000000) currentState.castleRights.blackQueenSide = false;
    if (to_bb & 0x8000000000000000) currentState.castleRights.blackKingSide = false;

    // Update en passant target
    currentState.enPassantTarget = std::nullopt;
    if (moving_piece_type == PieceType::PAWN && abs(move.start.rank - move.end.rank) == 2) {
        currentState.enPassantTarget = Square{(move.start.rank + move.end.rank) / 2, move.start.file};
    }

    // Update clocks
    bool is_capture = (to_bb & currentState.all_pieces);
    if (moving_piece_type == PieceType::PAWN || is_capture) { currentState.halfmoveClock = 0; } 
    else { currentState.halfmoveClock++; }
    if (currentState.currentTurn == Color::BLACK) { currentState.fullmoveNumber++; }

    // Switch turn
    currentState.currentTurn = (currentState.currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // Update composite bitboards
    updateCompositeBitboards();

    // --- Zobrist Update Part 6: Add new state ---
    const int newCastleRights = (currentState.castleRights.whiteKingSide << 3) |
                              (currentState.castleRights.whiteQueenSide << 2) |
                              (currentState.castleRights.blackKingSide << 1) |
                              (currentState.castleRights.blackQueenSide);
    hash ^= Zobrist::castleKeys[newCastleRights];
    if (currentState.enPassantTarget) {
        hash ^= Zobrist::enPassantKeys[currentState.enPassantTarget->file];
    }
    currentState.zobristKey = hash;
}

void BitboardRepresentation::makeNullMove() {
    // --- Zobrist Update: Remove old state ---
    uint64_t hash = currentState.zobristKey;
    hash ^= Zobrist::blackToMoveKey; // Always switch side to move
    if (currentState.enPassantTarget) {
        hash ^= Zobrist::enPassantKeys[currentState.enPassantTarget->file];
    }

    // --- Update Game State ---
    currentState.currentTurn = (currentState.currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    currentState.enPassantTarget = std::nullopt;
    currentState.halfmoveClock++; // A null move is a half-move

    // --- Zobrist Update: Add new state ---
    // No new en passant target, so no key to add
    currentState.zobristKey = hash;
}

std::string BitboardRepresentation::toFen() const {
    std::stringstream fen;
    for (int rank = 7; rank >= 0; --rank) {
        int empty_squares = 0;
        for (int file = 0; file < 8; ++file) {
            Square sq = {rank, file};
            Bitboard bb = BitboardUtils::squareToBitboard(sq);
            char piece_char = 0;
            if (currentState.pawn[0] & bb) piece_char = 'P';
            else if (currentState.knight[0] & bb) piece_char = 'N';
            else if (currentState.bishop[0] & bb) piece_char = 'B';
            else if (currentState.rook[0] & bb) piece_char = 'R';
            else if (currentState.queen[0] & bb) piece_char = 'Q';
            else if (currentState.king[0] & bb) piece_char = 'K';
            else if (currentState.pawn[1] & bb) piece_char = 'p';
            else if (currentState.knight[1] & bb) piece_char = 'n';
            else if (currentState.bishop[1] & bb) piece_char = 'b';
            else if (currentState.rook[1] & bb) piece_char = 'r';
            else if (currentState.queen[1] & bb) piece_char = 'q';
            else if (currentState.king[1] & bb) piece_char = 'k';

            if (piece_char != 0) {
                if (empty_squares > 0) {
                    fen << empty_squares;
                    empty_squares = 0;
                }
                fen << piece_char;
            } else {
                empty_squares++;
            }
        }
        if (empty_squares > 0) {
            fen << empty_squares;
        }
        if (rank > 0) {
            fen << '/';
        }
    }

    fen << ' ' << (currentState.currentTurn == Color::WHITE ? 'w' : 'b');

    std::string castling_rights;
    if (currentState.castleRights.whiteKingSide) castling_rights += 'K';
    if (currentState.castleRights.whiteQueenSide) castling_rights += 'Q';
    if (currentState.castleRights.blackKingSide) castling_rights += 'k';
    if (currentState.castleRights.blackQueenSide) castling_rights += 'q';
    fen << ' ' << (castling_rights.empty() ? "-" : castling_rights);

    if (currentState.enPassantTarget) {
        fen << ' ' << (char)('a' + currentState.enPassantTarget->file) << (char)('1' + currentState.enPassantTarget->rank);
    } else {
        fen << " -";
    }

    fen << ' ' << currentState.halfmoveClock << ' ' << currentState.fullmoveNumber;

    return fen.str();
}


namespace BitboardUtils {
    bool isSquareAttackedBy(Square s, Color attackerColor, const BoardState& boardState) {
        Bitboard target_bb = BitboardUtils::squareToBitboard(s);
        int attacker_idx = (attackerColor == Color::WHITE) ? 0 : 1;

        // Pawn attacks
        if (attackerColor == Color::WHITE) {
            if (((target_bb >> 7) & not_a_file) & boardState.pawn[attacker_idx]) return true;
            if (((target_bb >> 9) & not_h_file) & boardState.pawn[attacker_idx]) return true;
        } else {
            if (((target_bb << 7) & not_h_file) & boardState.pawn[attacker_idx]) return true;
            if (((target_bb << 9) & not_a_file) & boardState.pawn[attacker_idx]) return true;
        }

        // Knight attacks
        Bitboard knight_attacks = 0ULL;
        knight_attacks |= (target_bb & not_h_file) << 17;   // Up 2, Right 1
        knight_attacks |= (target_bb & not_a_file) << 15;   // Up 2, Left 1
        knight_attacks |= (target_bb & not_gh_file) << 10;  // Up 1, Right 2
        knight_attacks |= (target_bb & not_ab_file) << 6;   // Up 1, Left 2
        knight_attacks |= (target_bb & not_a_file) >> 17;   // Down 2, Left 1
        knight_attacks |= (target_bb & not_h_file) >> 15;   // Down 2, Right 1
        knight_attacks |= (target_bb & not_ab_file) >> 10;  // Down 1, Left 2
        knight_attacks |= (target_bb & not_gh_file) >> 6;   // Down 1, Right 2

        if (knight_attacks & boardState.knight[attacker_idx]) return true;

        // King attacks
        Bitboard king_attacks = 0ULL;
        king_attacks |= (target_bb & not_h_file) << 1; // Right
        king_attacks |= (target_bb & not_a_file) >> 1; // Left
        king_attacks |= (target_bb << 8); // Up
        king_attacks |= (target_bb >> 8); // Down
        king_attacks |= (target_bb & not_h_file) << 9; // Up-Right
        king_attacks |= (target_bb & not_a_file) << 7; // Up-Left
        king_attacks |= (target_bb & not_h_file) >> 7; // Down-Right
        king_attacks |= (target_bb & not_a_file) >> 9; // Down-Left
        if (king_attacks & boardState.king[attacker_idx]) return true;

        // Sliding piece attacks (Rook, Bishop, Queen)
        Bitboard rooks_and_queens = boardState.rook[attacker_idx] | boardState.queen[attacker_idx];
        if ((MagicBitboards::get_rook_attacks(s, boardState.all_pieces) & rooks_and_queens) != 0) {
            return true;
        }

        Bitboard bishops_and_queens = boardState.bishop[attacker_idx] | boardState.queen[attacker_idx];
        if ((MagicBitboards::get_bishop_attacks(s, boardState.all_pieces) & bishops_and_queens) != 0) {
            return true;
        }

        return false;
    }
}
