#include "../include/AIPlayer.h"
#include "../include/Game.h"
#include "../include/MoveGen.h"

#include <limits> // For std::numeric_limits
#include <vector> // For std::vector
#include <algorithm> // For std::max, std::min
#include <map> // For std::map


const std::array<int, 64> AIPlayer::pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};
const std::array<int, 64> AIPlayer::knight_pst = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};
const std::array<int, 64> AIPlayer::bishop_pst = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};
const std::array<int, 64> AIPlayer::rook_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};
const std::array<int, 64> AIPlayer::queen_pst = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};
const std::array<int, 64> AIPlayer::king_pst_mid = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

int AIPlayer::evaluate(const BoardState& boardState) {
    int score = 0;

    // Piece values map
    std::map<PieceType, int> pieceValues = {
        {PieceType::PAWN, 100},
        {PieceType::KNIGHT, 320},
        {PieceType::BISHOP, 330},
        {PieceType::ROOK, 500},
        {PieceType::QUEEN, 900},
        {PieceType::KING, 20000}
    };

    // 1. Material and Piece-Square Table Evaluation
    for (int color_idx = 0; color_idx < 2; ++color_idx) {
        Color color = (color_idx == 0) ? Color::WHITE : Color::BLACK;
        int sign = (color == Color::WHITE) ? 1 : -1;

        Bitboard pawns = boardState.pawn[color_idx];
        score += __builtin_popcountll(pawns) * pieceValues[PieceType::PAWN] * sign;
        for (int i = 0; i < 64; ++i) { if ((pawns >> i) & 1) score += pawn_pst[i] * sign; }

        Bitboard knights = boardState.knight[color_idx];
        score += __builtin_popcountll(knights) * pieceValues[PieceType::KNIGHT] * sign;
        for (int i = 0; i < 64; ++i) { if ((knights >> i) & 1) score += knight_pst[i] * sign; }

        Bitboard bishops = boardState.bishop[color_idx];
        score += __builtin_popcountll(bishops) * pieceValues[PieceType::BISHOP] * sign;
        for (int i = 0; i < 64; ++i) { if ((bishops >> i) & 1) score += bishop_pst[i] * sign; }

        Bitboard rooks = boardState.rook[color_idx];
        score += __builtin_popcountll(rooks) * pieceValues[PieceType::ROOK] * sign;
        for (int i = 0; i < 64; ++i) { if ((rooks >> i) & 1) score += rook_pst[i] * sign; }

        Bitboard queens = boardState.queen[color_idx];
        score += __builtin_popcountll(queens) * pieceValues[PieceType::QUEEN] * sign;
        for (int i = 0; i < 64; ++i) { if ((queens >> i) & 1) score += queen_pst[i] * sign; }
        
        Bitboard king = boardState.king[color_idx];
        for (int i = 0; i < 64; ++i) { if ((king >> i) & 1) score += king_pst_mid[i] * sign; }
    }

    // 2. Mobility Evaluation
    std::vector<Move> white_moves, black_moves;
    BoardState tempState = boardState;
    tempState.currentTurn = Color::WHITE;
    MoveGen::generateMoves(tempState, white_moves);
    tempState.currentTurn = Color::BLACK;
    MoveGen::generateMoves(tempState, black_moves);
    score += (white_moves.size() - black_moves.size()) * 5;

    // 3. King Safety Evaluation
    int king_safety_score = 0;
    // Castling bonus (simplified: check if king is on castled square)
    if (boardState.king[0] & 0x40) king_safety_score += 50; // White kingside castled (g1)
    if (boardState.king[0] & 0x4) king_safety_score += 50; // White queenside castled (c1)
    if (boardState.king[1] & (1ULL << 62)) king_safety_score -= 50; // Black kingside castled (g8)
    if (boardState.king[1] & (1ULL << 58)) king_safety_score -= 50; // Black queenside castled (c8)

    // Pawn shield (simplified: check pawns in front of king)
    // For white king
    int white_king_sq = __builtin_ctzll(boardState.king[0]);
    int white_king_rank = white_king_sq / 8;
    int white_king_file = white_king_sq % 8;
    if (white_king_rank < 7) { // Not on 8th rank
        for (int f_offset = -1; f_offset <= 1; ++f_offset) {
            if (white_king_file + f_offset >= 0 && white_king_file + f_offset < 8) {
                if (BitboardUtils::squareToBitboard({white_king_rank + 1, white_king_file + f_offset}) & boardState.pawn[0]) {
                    king_safety_score += 10;
                }
            }
        }
    }
    // For black king
    int black_king_sq = __builtin_ctzll(boardState.king[1]);
    int black_king_rank = black_king_sq / 8;
    int black_king_file = black_king_sq % 8;
    if (black_king_rank > 0) { // Not on 1st rank
        for (int f_offset = -1; f_offset <= 1; ++f_offset) {
            if (black_king_file + f_offset >= 0 && black_king_file + f_offset < 8) {
                if (BitboardUtils::squareToBitboard({black_king_rank - 1, black_king_file + f_offset}) & boardState.pawn[1]) {
                    king_safety_score -= 10;
                }
            }
        }
    }
    score += king_safety_score;

    // 4. Pawn Structure Evaluation
    int pawn_structure_score = 0;
    // Doubled pawns
    for (int f = 0; f < 8; ++f) {
        Bitboard file_pawns_white = boardState.pawn[0] & (0x0101010101010101ULL << f);
        Bitboard file_pawns_black = boardState.pawn[1] & (0x0101010101010101ULL << f);
        if (__builtin_popcountll(file_pawns_white) > 1) pawn_structure_score -= 20; // Penalty for doubled white pawns
        if (__builtin_popcountll(file_pawns_black) > 1) pawn_structure_score += 20; // Penalty for doubled black pawns
    }
    // Isolated pawns
    for (int f = 0; f < 8; ++f) {
        Bitboard current_file = (0x0101010101010101ULL << f);
        Bitboard adjacent_files = 0ULL;
        if (f > 0) adjacent_files |= (0x0101010101010101ULL << (f - 1));
        if (f < 7) adjacent_files |= (0x0101010101010101ULL << (f + 1));

        if ((boardState.pawn[0] & current_file) && ! (boardState.pawn[0] & adjacent_files)) pawn_structure_score -= 15; // Isolated white pawn
        if ((boardState.pawn[1] & current_file) && ! (boardState.pawn[1] & adjacent_files)) pawn_structure_score += 15; // Isolated black pawn
    }
    // Passed pawns (simplified: no opponent pawns in front on same or adjacent files)
    for (int i = 0; i < 64; ++i) {
        if ((boardState.pawn[0] >> i) & 1) { // White pawn
            int rank = i / 8;
            int file = i % 8;
            Bitboard passed_mask = 0ULL;
            for (int r = rank + 1; r < 8; ++r) {
                passed_mask |= (0x0101010101010101ULL << file);
                if (file > 0) passed_mask |= (0x0101010101010101ULL << (file - 1));
                if (file < 7) passed_mask |= (0x0101010101010101ULL << (file + 1));
            }
            if (! (boardState.pawn[1] & passed_mask)) pawn_structure_score += (rank * 10); // Bonus for passed white pawn
        }
        if ((boardState.pawn[1] >> i) & 1) { // Black pawn
            int rank = i / 8;
            int file = i % 8;
            Bitboard passed_mask = 0ULL;
            for (int r = rank - 1; r >= 0; --r) {
                passed_mask |= (0x0101010101010101ULL << file);
                if (file > 0) passed_mask |= (0x0101010101010101ULL << (file - 1));
                if (file < 7) passed_mask |= (0x0101010101010101ULL << (file + 1));
            }
            if (! (boardState.pawn[0] & passed_mask)) pawn_structure_score -= ( (7 - rank) * 10); // Bonus for passed black pawn
        }
    }
    score += pawn_structure_score;

    // 5. Center Control Evaluation
    Bitboard center = 0x00003C3C3C3C0000ULL; // d3,e3,f3,d4,e4,f4,d5,e5,f5,d6,e6,f6
    score += (__builtin_popcountll(boardState.white_pieces & center) - __builtin_popcountll(boardState.black_pieces & center)) * 5;

    // 6. Attacked Pieces Evaluation
    int attack_score = 0;
    const PieceType piece_types[] = {PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING};

    // White pieces attacked by Black
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
            if (BitboardUtils::isSquareAttackedBy({sq_idx / 8, sq_idx % 8}, Color::BLACK, boardState)) {
                attack_score -= pieceValues[piece_types[i]] / 4; // Penalty for white piece being attacked
            }
            pieces &= pieces - 1;
        }
    }

    // Black pieces attacked by White
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
            if (BitboardUtils::isSquareAttackedBy({sq_idx / 8, sq_idx % 8}, Color::WHITE, boardState)) {
                attack_score += pieceValues[piece_types[i]] / 4; // Bonus for white if it attacks a black piece
            }
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
        return evaluate(game.getBoard().getState());
    }
    if (depth == 0) {
        return quiescenceSearch(game, alpha, beta);
    }

    std::vector<Move> legalMoves = game.generateAllLegalMoves();

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& move : legalMoves) {
            Game tempGame = game;
            tempGame.makeMove(move);
            int eval = minimax(tempGame, depth - 1, !maximizingPlayer, alpha, beta);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const auto& move : legalMoves) {
            Game tempGame = game;
            tempGame.makeMove(move);
            int eval = minimax(tempGame, depth - 1, !maximizingPlayer, alpha, beta);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

Move AIPlayer::findBestMove(Game& game, int max_depth) {
    Move bestMove;
    std::vector<Move> principal_variation;

    bool isMaximizingPlayer = (game.getBoard().getState().currentTurn == Color::WHITE);

    for (int current_depth = 1; current_depth <= max_depth; ++current_depth) {
        int bestValue = isMaximizingPlayer ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
        Move currentBestMove;

        std::vector<Move> legalMoves = game.generateAllLegalMoves();

        // Sort moves to prioritize the best move from the previous iteration
        if (!principal_variation.empty()) {
            auto pv_move = principal_variation.front();
            for (size_t i = 0; i < legalMoves.size(); ++i) {
                if (legalMoves[i] == pv_move) {
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
        // For a real PV, we would need to reconstruct it from the search.
        // For now, we just store the best move at this depth.
        principal_variation.clear();
        principal_variation.push_back(bestMove);
    }
    return bestMove;
}