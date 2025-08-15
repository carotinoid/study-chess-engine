#include "../include/Zobrist.h"
#include <random>

namespace Zobrist {

    // Define the extern variables
    std::array<std::array<uint64_t, 64>, 12> pieceKeys;
    uint64_t blackToMoveKey;
    std::array<uint64_t, 16> castleKeys;
    std::array<uint64_t, 8> enPassantKeys;

    void Init() {
        std::mt19937_64 gen(0xDEADBEEFCAFEBABE); // Fixed seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist;

        for (int piece = 0; piece < 12; ++piece) {
            for (int square = 0; square < 64; ++square) {
                pieceKeys[piece][square] = dist(gen);
            }
        }

        blackToMoveKey = dist(gen);

        for (int i = 0; i < 16; ++i) {
            castleKeys[i] = dist(gen);
        }

        for (int i = 0; i < 8; ++i) {
            enPassantKeys[i] = dist(gen);
        }
    }

    uint64_t ComputeHash(const BoardState& boardState) {
        uint64_t hash = 0;

        // Piece positions
        for (int color_idx = 0; color_idx < 2; ++color_idx) {
            for (int pt_idx = 0; pt_idx < 6; ++pt_idx) {
                Bitboard bb = 0;
                switch(static_cast<PieceType>(pt_idx)) {
                    case PieceType::PAWN:   bb = boardState.pawn[color_idx]; break;
                    case PieceType::KNIGHT: bb = boardState.knight[color_idx]; break;
                    case PieceType::BISHOP: bb = boardState.bishop[color_idx]; break;
                    case PieceType::ROOK:   bb = boardState.rook[color_idx]; break;
                    case PieceType::QUEEN:  bb = boardState.queen[color_idx]; break;
                    case PieceType::KING:   bb = boardState.king[color_idx]; break;
                }
                int piece_idx = color_idx * 6 + pt_idx;
                while (bb) {
                    int sq = __builtin_ctzll(bb);
                    hash ^= pieceKeys[piece_idx][sq];
                    bb &= bb - 1;
                }
            }
        }

        // Side to move
        if (boardState.currentTurn == Color::BLACK) {
            hash ^= blackToMoveKey;
        }

        // Castling rights
        int castle_idx = (boardState.castleRights.whiteKingSide << 3) |
                         (boardState.castleRights.whiteQueenSide << 2) |
                         (boardState.castleRights.blackKingSide << 1) |
                         (boardState.castleRights.blackQueenSide);
        hash ^= castleKeys[castle_idx];

        // En passant
        if (boardState.enPassantTarget) {
            hash ^= enPassantKeys[boardState.enPassantTarget->file];
        }

        return hash;
    }

} // namespace Zobrist
