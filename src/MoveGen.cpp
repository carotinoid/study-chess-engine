#include "../include/MoveGen.h"
#include "../include/MagicBitboards.h"

namespace MoveGen {

    void generateMoves(const BoardState& boardState, std::vector<Move>& moves) {
        generatePawnMoves(boardState, moves);
        generateKnightMoves(boardState, moves);
        generateBishopMoves(boardState, moves);
        generateRookMoves(boardState, moves);
        generateQueenMoves(boardState, moves);
        generateKingMoves(boardState, moves);
    }

    void generateCaptureMoves(const BoardState& boardState, std::vector<Move>& moves) {
        int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
        Bitboard opponent_pieces = (color_idx == 0) ? boardState.black_pieces : boardState.white_pieces;
        Bitboard all_pieces = boardState.all_pieces;

        // Pawn captures
        Bitboard pawns = boardState.pawn[color_idx];
        if (boardState.currentTurn == Color::WHITE) {
            Bitboard left_captures = (pawns << 7) & opponent_pieces & ~0x8080808080808080ULL;
            Bitboard right_captures = (pawns << 9) & opponent_pieces & ~0x0101010101010101ULL;
            for (int i = 0; i < 64; ++i) {
                if ((left_captures >> i) & 1) {
                     if (i / 8 == 7) { // Promotion
                        moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::QUEEN});
                        moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::ROOK});
                        moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::BISHOP});
                        moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                    } else {
                        moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}});
                    }
                }
                if ((right_captures >> i) & 1) {
                    if (i / 8 == 7) { // Promotion
                        moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::QUEEN});
                        moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::ROOK});
                        moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::BISHOP});
                        moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                    } else {
                        moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}});
                    }
                }
            }
        } else { // Black
            Bitboard left_captures = (pawns >> 9) & opponent_pieces & ~0x8080808080808080ULL;
            Bitboard right_captures = (pawns >> 7) & opponent_pieces & ~0x0101010101010101ULL;
            for (int i = 0; i < 64; ++i) {
                if ((left_captures >> i) & 1) {
                    if (i / 8 == 0) { // Promotion
                        moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::QUEEN});
                        moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::ROOK});
                        moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::BISHOP});
                        moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                    } else {
                        moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}});
                    }
                }
                if ((right_captures >> i) & 1) {
                    if (i / 8 == 0) { // Promotion
                        moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::QUEEN});
                        moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::ROOK});
                        moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::BISHOP});
                        moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                    } else {
                        moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}});
                    }
                }
            }
        }
        // En passant
        if (boardState.enPassantTarget.has_value()) {
            int ep_sq = boardState.enPassantTarget->rank * 8 + boardState.enPassantTarget->file;
            Bitboard ep_bb = 1ULL << ep_sq;
            if (boardState.currentTurn == Color::WHITE) {
                Bitboard attackers = ((ep_bb >> 7) & ~0x8080808080808080ULL & pawns) | ((ep_bb >> 9) & ~0x0101010101010101ULL & pawns);
                if (attackers) moves.push_back({Square{__builtin_ctzll(attackers)/8, __builtin_ctzll(attackers)%8}, *boardState.enPassantTarget});
            } else {
                Bitboard attackers = ((ep_bb << 7) & ~0x0101010101010101ULL & pawns) | ((ep_bb << 9) & ~0x8080808080808080ULL & pawns);
                if (attackers) moves.push_back({Square{__builtin_ctzll(attackers)/8, __builtin_ctzll(attackers)%8}, *boardState.enPassantTarget});
            }
        }

        // Knight captures
        Bitboard knights = boardState.knight[color_idx];
        Bitboard not_a_file = ~0x0101010101010101ULL;
        Bitboard not_ab_file = ~0x0303030303030303ULL;
        Bitboard not_h_file = ~0x8080808080808080ULL;
        Bitboard not_gh_file = ~0xC0C0C0C0C0C0C0C0ULL;
        while(knights) {
            int from_sq = __builtin_ctzll(knights);
            Bitboard from_bb = 1ULL << from_sq;
            Bitboard attacks = 0ULL;
            attacks |= (from_bb & not_h_file) << 17;
            attacks |= (from_bb & not_a_file) << 15;
            attacks |= (from_bb & not_gh_file) << 10;
            attacks |= (from_bb & not_ab_file) << 6;
            attacks |= (from_bb & not_a_file) >> 17;
            attacks |= (from_bb & not_h_file) >> 15;
            attacks |= (from_bb & not_ab_file) >> 10;
            attacks |= (from_bb & not_gh_file) >> 6;
            attacks &= opponent_pieces;
            while(attacks) {
                int to_sq = __builtin_ctzll(attacks);
                moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
                attacks &= attacks - 1;
            }
            knights &= knights - 1;
        }

        // Bishop captures
        Bitboard bishops = boardState.bishop[color_idx];
        while(bishops) {
            int from_sq = __builtin_ctzll(bishops);
            Bitboard attacks = MagicBitboards::get_bishop_attacks({from_sq/8, from_sq%8}, all_pieces) & opponent_pieces;
            while(attacks) {
                int to_sq = __builtin_ctzll(attacks);
                moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
                attacks &= attacks - 1;
            }
            bishops &= bishops - 1;
        }

        // Rook captures
        Bitboard rooks = boardState.rook[color_idx];
        while(rooks) {
            int from_sq = __builtin_ctzll(rooks);
            Bitboard attacks = MagicBitboards::get_rook_attacks({from_sq/8, from_sq%8}, all_pieces) & opponent_pieces;
            while(attacks) {
                int to_sq = __builtin_ctzll(attacks);
                moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
                attacks &= attacks - 1;
            }
            rooks &= rooks - 1;
        }

        // Queen captures
        Bitboard queens = boardState.queen[color_idx];
        while(queens) {
            int from_sq = __builtin_ctzll(queens);
            Bitboard attacks = MagicBitboards::get_queen_attacks({from_sq/8, from_sq%8}, all_pieces) & opponent_pieces;
            while(attacks) {
                int to_sq = __builtin_ctzll(attacks);
                moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
                attacks &= attacks - 1;
            }
            queens &= queens - 1;
        }

        // King captures
        Bitboard king = boardState.king[color_idx];
        int from_sq = __builtin_ctzll(king);
        Bitboard from_bb = 1ULL << from_sq;
        Bitboard attacks = 0ULL;
        attacks |= (from_bb & not_h_file) << 1; // Right
        attacks |= (from_bb & not_a_file) >> 1; // Left
        attacks |= (from_bb << 8); // Up
        attacks |= (from_bb >> 8); // Down
        attacks |= (from_bb & not_h_file) << 9; // Up-Right
        attacks |= (from_bb & not_a_file) << 7; // Up-Left
        attacks |= (from_bb & not_h_file) >> 7; // Down-Right
        attacks |= (from_bb & not_a_file) >> 9; // Down-Left
        attacks &= opponent_pieces;
        while(attacks) {
            int to_sq = __builtin_ctzll(attacks);
            moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
            attacks &= attacks - 1;
        }
    }

    void generatePawnMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard pawns = boardState.pawn[color_idx];
    Bitboard opponent_pieces = (color_idx == 0) ? boardState.black_pieces : boardState.white_pieces;
    Bitboard all_pieces = boardState.all_pieces;

    // Single and double pushes
    if (boardState.currentTurn == Color::WHITE) {
        Bitboard single_push = (pawns << 8) & ~all_pieces;
        Bitboard double_push = ((single_push & 0x0000000000FF0000ULL) << 8) & ~all_pieces;
        for (int i = 0; i < 64; ++i) {
            if ((single_push >> i) & 1) {
                if (i / 8 == 7) { // Promotion
                    moves.push_back({Square{i/8 - 1, i%8}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 - 1, i%8}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 - 1, i%8}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 - 1, i%8}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 - 1, i%8}, Square{i/8, i%8}});
                }
            }
            if ((double_push >> i) & 1) moves.push_back({Square{i/8 - 2, i%8}, Square{i/8, i%8}});
        }
    } else { // Black
        Bitboard single_push = (pawns >> 8) & ~all_pieces;
        Bitboard double_push = ((single_push & 0x0000FF0000000000ULL) >> 8) & ~all_pieces;
        for (int i = 0; i < 64; ++i) {
            if ((single_push >> i) & 1) {
                if (i / 8 == 0) { // Promotion
                    moves.push_back({Square{i/8 + 1, i%8}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 + 1, i%8}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 + 1, i%8}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 + 1, i%8}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 + 1, i%8}, Square{i/8, i%8}});
                }
            }
            if ((double_push >> i) & 1) moves.push_back({Square{i/8 + 2, i%8}, Square{i/8, i%8}});
        }
    }

    // Captures
    if (boardState.currentTurn == Color::WHITE) {
        Bitboard left_captures = (pawns << 7) & opponent_pieces & ~0x8080808080808080ULL;
        Bitboard right_captures = (pawns << 9) & opponent_pieces & ~0x0101010101010101ULL;
        for (int i = 0; i < 64; ++i) {
            if ((left_captures >> i) & 1) {
                 if (i / 8 == 7) { // Promotion
                    moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 - 1, i%8 + 1}, Square{i/8, i%8}});
                }
            }
            if ((right_captures >> i) & 1) {
                if (i / 8 == 7) { // Promotion
                    moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 - 1, i%8 - 1}, Square{i/8, i%8}});
                }
            }
        }
    } else { // Black
        Bitboard left_captures = (pawns >> 9) & opponent_pieces & ~0x8080808080808080ULL;
        Bitboard right_captures = (pawns >> 7) & opponent_pieces & ~0x0101010101010101ULL;
        for (int i = 0; i < 64; ++i) {
            if ((left_captures >> i) & 1) {
                if (i / 8 == 0) { // Promotion
                    moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 + 1, i%8 + 1}, Square{i/8, i%8}});
                }
            }
            if ((right_captures >> i) & 1) {
                if (i / 8 == 0) { // Promotion
                    moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::QUEEN});
                    moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::ROOK});
                    moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::BISHOP});
                    moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}, PieceType::KNIGHT});
                } else {
                    moves.push_back({Square{i/8 + 1, i%8 - 1}, Square{i/8, i%8}});
                }
            }
        }
    }

    // En passant
    if (boardState.enPassantTarget.has_value()) {
        int ep_sq = boardState.enPassantTarget->rank * 8 + boardState.enPassantTarget->file;
        Bitboard ep_bb = 1ULL << ep_sq;
        if (boardState.currentTurn == Color::WHITE) {
            Bitboard attackers = ((ep_bb >> 7) & ~0x8080808080808080ULL & pawns) | ((ep_bb >> 9) & ~0x0101010101010101ULL & pawns);
            if (attackers) moves.push_back({Square{__builtin_ctzll(attackers)/8, __builtin_ctzll(attackers)%8}, *boardState.enPassantTarget});
        } else {
            Bitboard attackers = ((ep_bb << 7) & ~0x0101010101010101ULL & pawns) | ((ep_bb << 9) & ~0x8080808080808080ULL & pawns);
            if (attackers) moves.push_back({Square{__builtin_ctzll(attackers)/8, __builtin_ctzll(attackers)%8}, *boardState.enPassantTarget});
        }
    }
}

    void generateKnightMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard knights = boardState.knight[color_idx];
    Bitboard friendly_pieces = (color_idx == 0) ? boardState.white_pieces : boardState.black_pieces;

    Bitboard not_a_file = ~0x0101010101010101ULL;
    Bitboard not_ab_file = ~0x0303030303030303ULL;
    Bitboard not_h_file = ~0x8080808080808080ULL;
    Bitboard not_gh_file = ~0xC0C0C0C0C0C0C0C0ULL;

    while(knights) {
        int from_sq = __builtin_ctzll(knights);
        Bitboard from_bb = 1ULL << from_sq;

        Bitboard attacks = 0ULL;
        attacks |= (from_bb & not_h_file) << 17;   // Up 2, Right 1
        attacks |= (from_bb & not_a_file) << 15;   // Up 2, Left 1
        attacks |= (from_bb & not_gh_file) << 10;  // Up 1, Right 2
        attacks |= (from_bb & not_ab_file) << 6;   // Up 1, Left 2
        attacks |= (from_bb & not_a_file) >> 17;   // Down 2, Left 1
        attacks |= (from_bb & not_h_file) >> 15;   // Down 2, Right 1
        attacks |= (from_bb & not_ab_file) >> 10;  // Down 1, Left 2
        attacks |= (from_bb & not_gh_file) >> 6;   // Down 1, Right 2

        attacks &= ~friendly_pieces;

        while(attacks) {
            int to_sq = __builtin_ctzll(attacks);
            moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
            attacks &= attacks - 1;
        }

        knights &= knights - 1;
    }
}

    void generateBishopMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard bishops = boardState.bishop[color_idx];
    Bitboard friendly_pieces = (color_idx == 0) ? boardState.white_pieces : boardState.black_pieces;
    Bitboard all_pieces = boardState.all_pieces;

    while(bishops) {
        int from_sq = __builtin_ctzll(bishops);
        Bitboard attacks = MagicBitboards::get_bishop_attacks({from_sq/8, from_sq%8}, all_pieces) & ~friendly_pieces;

        while(attacks) {
            int to_sq = __builtin_ctzll(attacks);
            moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
            attacks &= attacks - 1;
        }

        bishops &= bishops - 1;
    }
}

    void generateRookMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard rooks = boardState.rook[color_idx];
    Bitboard friendly_pieces = (color_idx == 0) ? boardState.white_pieces : boardState.black_pieces;
    Bitboard all_pieces = boardState.all_pieces;

    while(rooks) {
        int from_sq = __builtin_ctzll(rooks);
        Bitboard attacks = MagicBitboards::get_rook_attacks({from_sq/8, from_sq%8}, all_pieces) & ~friendly_pieces;

        while(attacks) {
            int to_sq = __builtin_ctzll(attacks);
            moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
            attacks &= attacks - 1;
        }

        rooks &= rooks - 1;
    }
}

    void generateQueenMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard queens = boardState.queen[color_idx];
    Bitboard friendly_pieces = (color_idx == 0) ? boardState.white_pieces : boardState.black_pieces;
    Bitboard all_pieces = boardState.all_pieces;

    while(queens) {
        int from_sq = __builtin_ctzll(queens);
        Bitboard attacks = MagicBitboards::get_queen_attacks({from_sq/8, from_sq%8}, all_pieces) & ~friendly_pieces;

        while(attacks) {
            int to_sq = __builtin_ctzll(attacks);
            moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
            attacks &= attacks - 1;
        }

        queens &= queens - 1;
    }
}

    void generateKingMoves(const BoardState& boardState, std::vector<Move>& moves) {
    int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
    Bitboard king = boardState.king[color_idx];
    Bitboard friendly_pieces = (color_idx == 0) ? boardState.white_pieces : boardState.black_pieces;

    int from_sq = __builtin_ctzll(king);
    Bitboard from_bb = 1ULL << from_sq;

    Bitboard not_a_file = ~0x0101010101010101ULL;
    Bitboard not_h_file = ~0x8080808080808080ULL;

    Bitboard attacks = 0ULL;
    attacks |= (from_bb & not_h_file) << 1; // Right
    attacks |= (from_bb & not_a_file) >> 1; // Left
    attacks |= (from_bb << 8); // Up
    attacks |= (from_bb >> 8); // Down
    attacks |= (from_bb & not_h_file) << 9; // Up-Right
    attacks |= (from_bb & not_a_file) << 7; // Up-Left
    attacks |= (from_bb & not_h_file) >> 7; // Down-Right
    attacks |= (from_bb & not_a_file) >> 9; // Down-Left

    attacks &= ~friendly_pieces;

    while(attacks) {
        int to_sq = __builtin_ctzll(attacks);
        moves.push_back({Square{from_sq/8, from_sq%8}, Square{to_sq/8, to_sq%8}});
        attacks &= attacks - 1;
    }

    // Castling
    if (boardState.currentTurn == Color::WHITE) {
        if (boardState.castleRights.whiteKingSide) {
            if (!((boardState.all_pieces & 0x60) || BitboardUtils::isSquareAttackedBy({0, 4}, Color::BLACK, boardState) || BitboardUtils::isSquareAttackedBy({0, 5}, Color::BLACK, boardState) || BitboardUtils::isSquareAttackedBy({0, 6}, Color::BLACK, boardState))) {
                moves.push_back({Square{0, 4}, Square{0, 6}});
            }
        }
        if (boardState.castleRights.whiteQueenSide) {
            if (!((boardState.all_pieces & 0xE) || BitboardUtils::isSquareAttackedBy({0, 4}, Color::BLACK, boardState) || BitboardUtils::isSquareAttackedBy({0, 3}, Color::BLACK, boardState) || BitboardUtils::isSquareAttackedBy({0, 2}, Color::BLACK, boardState))) {
                moves.push_back({Square{0, 4}, Square{0, 2}});
            }
        }
    } else {
        if (boardState.castleRights.blackKingSide) {
            if (!((boardState.all_pieces & 0x6000000000000000) || BitboardUtils::isSquareAttackedBy({7, 4}, Color::WHITE, boardState) || BitboardUtils::isSquareAttackedBy({7, 5}, Color::WHITE, boardState) || BitboardUtils::isSquareAttackedBy({7, 6}, Color::WHITE, boardState))) {
                moves.push_back({Square{7, 4}, Square{7, 6}});
            }
        }
        if (boardState.castleRights.blackQueenSide) {
            if (!((boardState.all_pieces & 0xE00000000000000) || BitboardUtils::isSquareAttackedBy({7, 4}, Color::WHITE, boardState) || BitboardUtils::isSquareAttackedBy({7, 3}, Color::WHITE, boardState) || BitboardUtils::isSquareAttackedBy({7, 2}, Color::WHITE, boardState))) {
                moves.push_back({Square{7, 4}, Square{7, 2}});
            }
        }
    }
}
}