#include "../include/Bitboard.h"
#include "../include/MoveGen.h"
#include "../include/MagicBitboards.h"
#include <iostream>

namespace {
    // Constants for file masks, to prevent wrap-around when generating moves
    constexpr Bitboard not_a_file = 0xfefefefefefefefeULL; // ~A1
    constexpr Bitboard not_h_file = 0x7f7f7f7f7f7f7f7fULL; // ~H1
    constexpr Bitboard not_ab_file = 0xfcfcfcfcfcfcfcfcULL; // ~A1 & ~B1
    constexpr Bitboard not_gh_file = 0x3f3f3f3f3f3f3f3fULL; // ~G1 & ~H1
}

BitboardRepresentation::BitboardRepresentation() {
    // Initialize all bitboards to 0
    for (int i = 0; i < 2; ++i) {
        currentState.pawn[i] = 0;
        currentState.knight[i] = 0;
        currentState.bishop[i] = 0;
        currentState.rook[i] = 0;
        currentState.queen[i] = 0;
        currentState.king[i] = 0;
    }
    updateCompositeBitboards();
    currentState.currentTurn = Color::WHITE;
    // Other initializations
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
    int us_idx = (currentState.currentTurn == Color::WHITE) ? 0 : 1;
    int them_idx = 1 - us_idx;
    Bitboard from_bb = BitboardUtils::squareToBitboard(move.start);
    Bitboard to_bb = BitboardUtils::squareToBitboard(move.end);
    Bitboard from_to_bb = from_bb | to_bb;

    // Find which piece is moving
    PieceType moving_piece_type = PieceType::PAWN; // Default
    if (from_bb & currentState.pawn[us_idx]) moving_piece_type = PieceType::PAWN;
    else if (from_bb & currentState.knight[us_idx]) moving_piece_type = PieceType::KNIGHT;
    else if (from_bb & currentState.bishop[us_idx]) moving_piece_type = PieceType::BISHOP;
    else if (from_bb & currentState.rook[us_idx]) moving_piece_type = PieceType::ROOK;
    else if (from_bb & currentState.queen[us_idx]) moving_piece_type = PieceType::QUEEN;
    else if (from_bb & currentState.king[us_idx]) moving_piece_type = PieceType::KING;

    bool is_capture = (to_bb & currentState.all_pieces);
    
    // --- Update Piece Bitboards ---
    
    // 1. Move the piece
    auto move_piece = [&](PieceType pt) {
        switch(pt) {
            case PieceType::PAWN:   currentState.pawn[us_idx] ^= from_to_bb; break;
            case PieceType::KNIGHT: currentState.knight[us_idx] ^= from_to_bb; break;
            case PieceType::BISHOP: currentState.bishop[us_idx] ^= from_to_bb; break;
            case PieceType::ROOK:   currentState.rook[us_idx] ^= from_to_bb; break;
            case PieceType::QUEEN:  currentState.queen[us_idx] ^= from_to_bb; break;
            case PieceType::KING:   currentState.king[us_idx] ^= from_to_bb; break;
        }
    };
    move_piece(moving_piece_type);

    // 2. Handle captures
    if (is_capture) {
        if (to_bb & currentState.pawn[them_idx]) currentState.pawn[them_idx] ^= to_bb;
        else if (to_bb & currentState.knight[them_idx]) currentState.knight[them_idx] ^= to_bb;
        else if (to_bb & currentState.bishop[them_idx]) currentState.bishop[them_idx] ^= to_bb;
        else if (to_bb & currentState.rook[them_idx]) currentState.rook[them_idx] ^= to_bb;
        else if (to_bb & currentState.queen[them_idx]) currentState.queen[them_idx] ^= to_bb;
    }

    // 3. Handle special moves
    // En Passant
    if (moving_piece_type == PieceType::PAWN && move.end == currentState.enPassantTarget) {
        Bitboard captured_pawn_bb = (currentState.currentTurn == Color::WHITE) ? (to_bb >> 8) : (to_bb << 8);
        currentState.pawn[them_idx] ^= captured_pawn_bb;
    }

    // Promotion
    if (move.promotionPiece.has_value()) {
        currentState.pawn[us_idx] ^= to_bb; // Remove the pawn from the promotion square
        move_piece(*move.promotionPiece);      // Add the new piece
    }

    // Castling
    if (moving_piece_type == PieceType::KING && abs(move.start.file - move.end.file) == 2) {
        Bitboard rook_from_bb, rook_to_bb;
        if (move.end.file == 6) { // Kingside
            rook_from_bb = BitboardUtils::squareToBitboard({move.start.rank, 7});
            rook_to_bb = BitboardUtils::squareToBitboard({move.start.rank, 5});
        } else { // Queenside
            rook_from_bb = BitboardUtils::squareToBitboard({move.start.rank, 0});
            rook_to_bb = BitboardUtils::squareToBitboard({move.start.rank, 3});
        }
        currentState.rook[us_idx] ^= (rook_from_bb | rook_to_bb);
    }

    // --- Update Game State ---

    // 1. Update castling rights
    if (moving_piece_type == PieceType::KING) {
        if (us_idx == 0) {
            currentState.castleRights.whiteKingSide = false;
            currentState.castleRights.whiteQueenSide = false;
        } else {
            currentState.castleRights.blackKingSide = false;
            currentState.castleRights.blackQueenSide = false;
        }
    }
    if (from_bb & 0x81) { // A1 or H1
        if (from_bb & 0x1) currentState.castleRights.whiteQueenSide = false;
        if (from_bb & 0x80) currentState.castleRights.whiteKingSide = false;
    }
    if (from_bb & 0x8100000000000000) { // A8 or H8
        if (from_bb & 0x0100000000000000) currentState.castleRights.blackQueenSide = false;
        if (from_bb & 0x8000000000000000) currentState.castleRights.blackKingSide = false;
    }
    if (to_bb & 0x81) { // Capture on A1 or H1
        if (to_bb & 0x1) currentState.castleRights.whiteQueenSide = false;
        if (to_bb & 0x80) currentState.castleRights.whiteKingSide = false;
    }
     if (to_bb & 0x8100000000000000) { // Capture on A8 or H8
        if (to_bb & 0x0100000000000000) currentState.castleRights.blackQueenSide = false;
        if (to_bb & 0x8000000000000000) currentState.castleRights.blackKingSide = false;
    }

    // 2. Update en passant target
    currentState.enPassantTarget = std::nullopt;
    if (moving_piece_type == PieceType::PAWN && abs(move.start.rank - move.end.rank) == 2) {
        currentState.enPassantTarget = Square{(move.start.rank + move.end.rank) / 2, move.start.file};
    }

    // 3. Update clocks
    if (moving_piece_type == PieceType::PAWN || is_capture) {
        currentState.halfmoveClock = 0;
    } else {
        currentState.halfmoveClock++;
    }
    if (currentState.currentTurn == Color::BLACK) {
        currentState.fullmoveNumber++;
    }

    // 4. Switch turn
    currentState.currentTurn = (currentState.currentTurn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // 5. Update composite bitboards
    updateCompositeBitboards();
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
