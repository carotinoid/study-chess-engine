#include "../include/AIPlayer.h"
#include "../include/Game.h"
#include "../include/MoveGen.h"
#include <iostream>

#include <limits> // For std::numeric_limits
#include <vector> // For std::vector
#include <algorithm> // For std::max, std::min
#include <map> // For std::map

namespace {
    // Helper struct for move ordering
    struct ScoredMove {
        Move move;
        int score;

        bool operator>(const ScoredMove& other) const {
            return score > other.score;
        }
    };

    // Helper to get piece value from its type
    int get_piece_value(PieceType type) {
        switch(type) {
            case PieceType::PAWN:   return AIPlayer::PAWN_VALUE;
            case PieceType::KNIGHT: return AIPlayer::KNIGHT_VALUE;
            case PieceType::BISHOP: return AIPlayer::BISHOP_VALUE;
            case PieceType::ROOK:   return AIPlayer::ROOK_VALUE;
            case PieceType::QUEEN:  return AIPlayer::QUEEN_VALUE;
            case PieceType::KING:   return AIPlayer::KING_VALUE;
        }
        return 0;
    }

    // Helper to get piece type at a square
    std::optional<PieceType> getPieceTypeAtSquare(const BoardState& boardState, Square s) {
        Bitboard bb = BitboardUtils::squareToBitboard(s);
        for (int i = 0; i < 2; ++i) {
            if (boardState.pawn[i] & bb) return PieceType::PAWN;
            if (boardState.knight[i] & bb) return PieceType::KNIGHT;
            if (boardState.bishop[i] & bb) return PieceType::BISHOP;
            if (boardState.rook[i] & bb) return PieceType::ROOK;
            if (boardState.queen[i] & bb) return PieceType::QUEEN;
            if (boardState.king[i] & bb) return PieceType::KING;
        }
        return std::nullopt;
    }
}

AIPlayer::AIPlayer() {
    // Initialize the transposition table with a size of 64 MB
    transpositionTable = std::make_unique<TranspositionTable>(64);
    openingBook = std::make_unique<OpeningBook>("opening/eco");
    openingBook->load();
}

int AIPlayer::evaluate(const BoardState& boardState) {
    int score = 0;

    // Tapered Evaluation - Game Phase Calculation
    int gamePhase = 0;
    gamePhase += __builtin_popcountll(boardState.knight[0] | boardState.knight[1]) * 1;
    gamePhase += __builtin_popcountll(boardState.bishop[0] | boardState.bishop[1]) * 1;
    gamePhase += __builtin_popcountll(boardState.rook[0] | boardState.rook[1]) * 2;
    gamePhase += __builtin_popcountll(boardState.queen[0] | boardState.queen[1]) * 4;
    gamePhase = std::min(24, gamePhase);

    int material_score = 0;
    int mg_pst_score = 0;
    int eg_pst_score = 0;
    int advanced_score = 0;

    // 1. Material, PST, and Advanced Evaluation
    for (int color_idx = 0; color_idx < 2; ++color_idx) {
        Color color = (color_idx == 0) ? Color::WHITE : Color::BLACK;
        int sign = (color == Color::WHITE) ? 1 : -1;

        material_score += __builtin_popcountll(boardState.pawn[color_idx]) * PAWN_VALUE * sign;
        material_score += __builtin_popcountll(boardState.knight[color_idx]) * KNIGHT_VALUE * sign;
        material_score += __builtin_popcountll(boardState.bishop[color_idx]) * BISHOP_VALUE * sign;
        material_score += __builtin_popcountll(boardState.rook[color_idx]) * ROOK_VALUE * sign;
        material_score += __builtin_popcountll(boardState.queen[color_idx]) * QUEEN_VALUE * sign;

        Bitboard pieces = boardState.pawn[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) { mg_pst_score += pawn_pst[i] * sign; eg_pst_score += pawn_pst_end[i] * sign; } }
        pieces = boardState.knight[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) mg_pst_score += knight_pst[i] * sign; }
        pieces = boardState.bishop[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) mg_pst_score += bishop_pst[i] * sign; }
        pieces = boardState.rook[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) mg_pst_score += rook_pst[i] * sign; }
        pieces = boardState.queen[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) mg_pst_score += queen_pst[i] * sign; }
        pieces = boardState.king[color_idx];
        for (int i = 0; i < 64; ++i) { if ((pieces >> i) & 1) { mg_pst_score += king_pst_mid[i] * sign; eg_pst_score += king_pst_end[i] * sign; } }

        if (__builtin_popcountll(boardState.bishop[color_idx]) >= 2) { advanced_score += BISHOP_PAIR_BONUS * sign; }

        Bitboard rooks = boardState.rook[color_idx];
        while(rooks) {
            int sq_idx = __builtin_ctzll(rooks);
            int file = sq_idx % 8;
            Bitboard file_mask = 0x0101010101010101ULL << file;
            bool friendly_pawns_on_file = (boardState.pawn[color_idx] & file_mask) != 0;
            bool opponent_pawns_on_file = (boardState.pawn[1 - color_idx] & file_mask) != 0;
            if (!friendly_pawns_on_file) {
                if (!opponent_pawns_on_file) { advanced_score += ROOK_ON_OPEN_FILE_BONUS * sign; }
                else { advanced_score += ROOK_ON_SEMI_OPEN_FILE_BONUS * sign; }
            }
            rooks &= rooks - 1;
        }
    }

    int final_pst_score = (mg_pst_score * gamePhase + eg_pst_score * (24 - gamePhase)) / 24;
    score += material_score + (final_pst_score * PST_WEIGHT) + advanced_score;

    int mobility_score = 0;
    std::vector<Move> white_moves, black_moves;
    BoardState tempState = boardState;
    tempState.currentTurn = Color::WHITE;
    MoveGen::generateMoves(tempState, white_moves);
    tempState.currentTurn = Color::BLACK;
    MoveGen::generateMoves(tempState, black_moves);
    mobility_score = (white_moves.size() - black_moves.size()) * MOBILITY_WEIGHT;
    score += mobility_score;

    int king_safety_score = 0;
    if (boardState.king[0] & 0x40) king_safety_score += KING_SAFETY_CASTLE_BONUS;
    if (boardState.king[0] & 0x4) king_safety_score += KING_SAFETY_CASTLE_BONUS;
    if (boardState.king[1] & (1ULL << 62)) king_safety_score -= KING_SAFETY_CASTLE_BONUS;
    if (boardState.king[1] & (1ULL << 58)) king_safety_score -= KING_SAFETY_CASTLE_BONUS;

    int white_king_sq = __builtin_ctzll(boardState.king[0]);
    int white_king_rank = white_king_sq / 8;
    int white_king_file = white_king_sq % 8;
    if (white_king_rank < 7) {
        for (int f_offset = -1; f_offset <= 1; ++f_offset) {
            if (white_king_file + f_offset >= 0 && white_king_file + f_offset < 8) {
                if (BitboardUtils::squareToBitboard({white_king_rank + 1, white_king_file + f_offset}) & boardState.pawn[0]) { king_safety_score += KING_SAFETY_PAWN_SHIELD_BONUS; }
            }
        }
    }
    int black_king_sq = __builtin_ctzll(boardState.king[1]);
    int black_king_rank = black_king_sq / 8;
    int black_king_file = black_king_sq % 8;
    if (black_king_rank > 0) {
        for (int f_offset = -1; f_offset <= 1; ++f_offset) {
            if (black_king_file + f_offset >= 0 && black_king_file + f_offset < 8) {
                if (BitboardUtils::squareToBitboard({black_king_rank - 1, black_king_file + f_offset}) & boardState.pawn[1]) { king_safety_score -= KING_SAFETY_PAWN_SHIELD_BONUS; }
            }
        }
    }
    score += king_safety_score;

    int pawn_structure_score = 0;
    for (int f = 0; f < 8; ++f) {
        Bitboard file_pawns_white = boardState.pawn[0] & (0x0101010101010101ULL << f);
        Bitboard file_pawns_black = boardState.pawn[1] & (0x0101010101010101ULL << f);
        if (__builtin_popcountll(file_pawns_white) > 1) pawn_structure_score -= PAWN_DOUBLED_PENALTY;
        if (__builtin_popcountll(file_pawns_black) > 1) pawn_structure_score += PAWN_DOUBLED_PENALTY;
    }
    for (int f = 0; f < 8; ++f) {
        Bitboard current_file = (0x0101010101010101ULL << f);
        Bitboard adjacent_files = 0ULL;
        if (f > 0) adjacent_files |= (0x0101010101010101ULL << (f - 1));
        if (f < 7) adjacent_files |= (0x0101010101010101ULL << (f + 1));
        if ((boardState.pawn[0] & current_file) && ! (boardState.pawn[0] & adjacent_files)) pawn_structure_score -= PAWN_ISOLATED_PENALTY;
        if ((boardState.pawn[1] & current_file) && ! (boardState.pawn[1] & adjacent_files)) pawn_structure_score += PAWN_ISOLATED_PENALTY;
    }
    for (int i = 0; i < 64; ++i) {
        if ((boardState.pawn[0] >> i) & 1) {
            int rank = i / 8; int file = i % 8;
            Bitboard passed_mask = 0ULL;
            for (int r = rank + 1; r < 8; ++r) { passed_mask |= (0x0101010101010101ULL << file); if (file > 0) passed_mask |= (0x0101010101010101ULL << (file - 1)); if (file < 7) passed_mask |= (0x0101010101010101ULL << (file + 1)); }
            if (! (boardState.pawn[1] & passed_mask)) pawn_structure_score += (rank * PAWN_PASSED_BONUS);
        }
        if ((boardState.pawn[1] >> i) & 1) {
            int rank = i / 8; int file = i % 8;
            Bitboard passed_mask = 0ULL;
            for (int r = rank - 1; r >= 0; --r) { passed_mask |= (0x0101010101010101ULL << file); if (file > 0) passed_mask |= (0x0101010101010101ULL << (file - 1)); if (file < 7) passed_mask |= (0x0101010101010101ULL << (file + 1)); }
            if (! (boardState.pawn[0] & passed_mask)) pawn_structure_score -= ( (7 - rank) * PAWN_PASSED_BONUS);
        }
    }
    score += pawn_structure_score;

    Bitboard center = 0x00003C3C3C3C0000ULL;
    score += (__builtin_popcountll(boardState.white_pieces & center) - __builtin_popcountll(boardState.black_pieces & center)) * CENTER_CONTROL_WEIGHT;

    int attack_score = 0;
    const PieceType piece_types[] = {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING};
    for (int i = 0; i < 6; ++i) {
        Bitboard pieces = 0;
        switch(piece_types[i]) {
            case PieceType::PAWN:   pieces = boardState.pawn[0]; break;
            case PieceType::KNIGHT: pieces = boardState.knight[0]; break;
            case PieceType::BISHOP: pieces = boardState.bishop[0]; break;
            case PieceType::ROOK:   pieces = boardState.rook[0]; break;
            case PieceType::QUEEN:  pieces = boardState.queen[0]; break;
            case PieceType::KING:   pieces = boardState.king[0]; break;
        }
        while(pieces) {
            int sq_idx = __builtin_ctzll(pieces);
            if (BitboardUtils::isSquareAttackedBy({sq_idx / 8, sq_idx % 8}, Color::BLACK, boardState)) { attack_score -= get_piece_value(piece_types[i]) / ATTACKED_PIECE_PENALTY_FACTOR; }
            pieces &= pieces - 1;
        }
    }
    for (int i = 0; i < 6; ++i) {
        Bitboard pieces = 0;
        switch(piece_types[i]) {
            case PieceType::PAWN:   pieces = boardState.pawn[1]; break;
            case PieceType::KNIGHT: pieces = boardState.knight[1]; break;
            case PieceType::BISHOP: pieces = boardState.bishop[1]; break;
            case PieceType::ROOK:   pieces = boardState.rook[1]; break;
            case PieceType::QUEEN:  pieces = boardState.queen[1]; break;
            case PieceType::KING:   pieces = boardState.king[1]; break;
        }
        while(pieces) {
            int sq_idx = __builtin_ctzll(pieces);
            if (BitboardUtils::isSquareAttackedBy({sq_idx / 8, sq_idx % 8}, Color::WHITE, boardState)) { attack_score += get_piece_value(piece_types[i]) / ATTACKED_PIECE_PENALTY_FACTOR; }
            pieces &= pieces - 1;
        }
    }
    score += attack_score;

    return score;
}

int AIPlayer::quiescenceSearch(Game& game, int alpha, int beta) {
    int stand_pat = evaluate(game.getBoard().getState());
    bool isMaximizingPlayer = (game.getBoard().getState().currentTurn == Color::WHITE);

    if (isMaximizingPlayer) {
        alpha = std::max(alpha, stand_pat);
    } else {
        beta = std::min(beta, stand_pat);
    }
    if (alpha >= beta) {
        return stand_pat;
    }

    std::vector<Move> captureMoves;
    MoveGen::generateCaptureMoves(game.getBoard().getState(), captureMoves);

    for (const auto& move : captureMoves) {
        Game tempGame = game;
        Color movingPlayerColor = tempGame.getBoard().getState().currentTurn;
        tempGame.makeMove(move);

        int king_color_idx = (movingPlayerColor == Color::WHITE) ? 0 : 1;
        Color opponentColor = (movingPlayerColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
        int king_sq = __builtin_ctzll(tempGame.getBoard().getState().king[king_color_idx]);
        if (BitboardUtils::isSquareAttackedBy({king_sq / 8, king_sq % 8}, opponentColor, tempGame.getBoard().getState())) {
            continue; // Skip illegal move
        }

        int score = quiescenceSearch(tempGame, alpha, beta);

        if (isMaximizingPlayer) {
            alpha = std::max(alpha, score);
        } else {
            beta = std::min(beta, score);
        }
        if (alpha >= beta) {
            break;
        }
    }
    return isMaximizingPlayer ? alpha : beta;
}

int AIPlayer::minimax(Game& game, int depth, bool maximizingPlayer, int alpha, int beta) {
    if (game.getStatus() != GameStatus::ONGOING) {
        if (game.getStatus() == GameStatus::CHECKMATE) {
            return maximizingPlayer ? -MATE_SCORE : MATE_SCORE;
        } else { // Stalemate or Draw
            return 0;
        }
    }

    uint64_t hash = game.getBoard().getState().zobristKey;
    TTEntry* ttEntry = transpositionTable->Probe(hash);
    Move tt_move{};

    if (ttEntry != nullptr && ttEntry->depth >= depth) {
        if (ttEntry->flag == NodeType::EXACT) {
            return ttEntry->score;
        }
        if (ttEntry->flag == NodeType::LOWER_BOUND) {
            alpha = std::max(alpha, ttEntry->score);
        } else if (ttEntry->flag == NodeType::UPPER_BOUND) {
            beta = std::min(beta, ttEntry->score);
        }
        if (alpha >= beta) {
            return ttEntry->score;
        }
        tt_move = ttEntry->bestMove;
    }

    if (depth == 0) {
        return quiescenceSearch(game, alpha, beta);
    }

    // --- Move Ordering ---
    std::vector<Move> legalMoves = game.generateAllLegalMoves();
    std::vector<ScoredMove> scoredMoves;
    const BoardState& boardState = game.getBoard().getState();

    for (const auto& move : legalMoves) {
        int score = 0;
        if (move == tt_move) {
            score = 20000; // Always search TT move first
        } else if (BitboardUtils::squareToBitboard(move.end) & boardState.all_pieces) { // Is capture?
            auto attacker = getPieceTypeAtSquare(boardState, move.start);
            auto victim = getPieceTypeAtSquare(boardState, move.end);
            if (attacker && victim) { // Should always be true for captures
                score = (get_piece_value(*victim) * 100) - get_piece_value(*attacker) + 10000;
            }
        } else {
            // Quiet moves - can be improved with killer/history heuristics later
            score = 0;
        }
        scoredMoves.push_back({move, score});
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<ScoredMove>());

    // --- Search Loop ---
    Move bestMove{};
    int bestValue;
    NodeType flag = NodeType::UPPER_BOUND; // Assume fail-low initially

    if (maximizingPlayer) {
        bestValue = std::numeric_limits<int>::min();
        for (const auto& scoredMove : scoredMoves) {
            Game tempGame = game;
            tempGame.makeMove(scoredMove.move);
            int eval = minimax(tempGame, depth - 1, !maximizingPlayer, alpha, beta);
            if (eval > bestValue) {
                bestValue = eval;
                bestMove = scoredMove.move;
            }
            if (bestValue >= beta) { // Fail-high
                bestValue = beta;
                flag = NodeType::LOWER_BOUND;
                break;
            }
            if (bestValue > alpha) {
                alpha = bestValue;
                flag = NodeType::EXACT;
            }
        }
    } else { // Minimizing player
        bestValue = std::numeric_limits<int>::max();
        for (const auto& scoredMove : scoredMoves) {
            Game tempGame = game;
            tempGame.makeMove(scoredMove.move);
            int eval = minimax(tempGame, depth - 1, !maximizingPlayer, alpha, beta);
            if (eval < bestValue) {
                bestValue = eval;
                bestMove = scoredMove.move;
            }
            if (bestValue <= alpha) { // Fail-low
                bestValue = alpha;
                flag = NodeType::UPPER_BOUND;
                break;
            }
            if (bestValue < beta) {
                beta = bestValue;
                flag = NodeType::EXACT;
            }
        }
    }

    transpositionTable->Store(hash, depth, bestValue, flag, bestMove);
    return bestValue;
}

Move AIPlayer::findBestMove(Game& game, int max_depth) {
    // Check opening book first
    std::vector<Move> bookMoves = openingBook->findMoves(game.getBoard().getState().zobristKey);
    if (!bookMoves.empty()) {
        // If there are multiple moves, pick one randomly
        // This adds variability to the opening play.
        int randomIndex = rand() % bookMoves.size();
        return bookMoves[randomIndex];
    }

    transpositionTable->Clear(); // Clear TT for new search
    Move bestMove;
    std::vector<Move> principal_variation;

    bool isMaximizingPlayer = (game.getBoard().getState().currentTurn == Color::WHITE);

    for (int current_depth = 1; current_depth <= max_depth; ++current_depth) {
        int bestValue = isMaximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
        Move currentBestMove;

        std::vector<Move> legalMoves = game.generateAllLegalMoves();

        // Sort moves using PV from previous iteration
        TTEntry* pv_entry = transpositionTable->Probe(game.getBoard().getState().zobristKey);
        if (pv_entry != nullptr) {
             for (size_t i = 0; i < legalMoves.size(); ++i) {
                if (legalMoves[i] == pv_entry->bestMove) {
                    std::swap(legalMoves[i], legalMoves[0]);
                    break;
                }
            }
        }

        for (const auto& move : legalMoves) {
            Game tempGame = game;
            tempGame.makeMove(move);
            
            int boardValue = minimax(tempGame, current_depth - 1, !isMaximizingPlayer, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

            if (isMaximizingPlayer) {
                if (boardValue > bestValue) {
                    bestValue = boardValue;
                    currentBestMove = move;
                }
            } else {
                if (boardValue < bestValue) {
                    bestValue = boardValue;
                    currentBestMove = move;
                }
            }
        }
        bestMove = currentBestMove;
    }
    return bestMove;
}