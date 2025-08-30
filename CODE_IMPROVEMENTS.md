# 구체적 코드 개선 제안서

## 🔧 즉시 적용 가능한 개선사항

### 1. 매직 넘버 제거 및 상수화

#### 현재 코드 (문제점)
```cpp
// MoveGen.cpp - 의미 불분명한 하드코딩
Bitboard left_captures = (pawns << 7) & opponent_pieces & ~0x8080808080808080ULL;
Bitboard right_captures = (pawns << 9) & opponent_pieces & ~0x0101010101010101ULL;
```

#### 개선된 코드
```cpp
// 새로운 BitboardConstants.h 파일 제안
#ifndef BITBOARD_CONSTANTS_H
#define BITBOARD_CONSTANTS_H

#include "DataTypes.h"

namespace BitboardConstants {
    // File masks
    constexpr Bitboard FILE_A = 0x0101010101010101ULL;
    constexpr Bitboard FILE_B = 0x0202020202020202ULL;
    constexpr Bitboard FILE_G = 0x4040404040404040ULL;
    constexpr Bitboard FILE_H = 0x8080808080808080ULL;
    
    // Rank masks
    constexpr Bitboard RANK_1 = 0x00000000000000FFULL;
    constexpr Bitboard RANK_2 = 0x000000000000FF00ULL;
    constexpr Bitboard RANK_7 = 0x00FF000000000000ULL;
    constexpr Bitboard RANK_8 = 0xFF00000000000000ULL;
    
    // Pawn capture masks
    constexpr Bitboard NOT_FILE_A = ~FILE_A;
    constexpr Bitboard NOT_FILE_H = ~FILE_H;
    
    // Center squares
    constexpr Bitboard CENTER_4 = 0x0000001818000000ULL; // e4, e5, d4, d5
    constexpr Bitboard EXTENDED_CENTER = 0x00003C3C3C3C0000ULL;
}

// 개선된 MoveGen.cpp
Bitboard left_captures = (pawns << 7) & opponent_pieces & BitboardConstants::NOT_FILE_H;
Bitboard right_captures = (pawns << 9) & opponent_pieces & BitboardConstants::NOT_FILE_A;
```

### 2. 폰 로직 중복 제거

#### 현재 문제점
`MoveGen.cpp`와 `Bitboard.cpp`에서 폰 공격 로직이 중복됨

#### 해결 방안: PawnUtils 클래스 생성
```cpp
// 새로운 PawnUtils.h 파일
#ifndef PAWN_UTILS_H
#define PAWN_UTILS_H

#include "DataTypes.h"
#include "BitboardConstants.h"

class PawnUtils {
public:
    // 폰의 공격 칸들 반환
    static Bitboard getPawnAttacks(Bitboard pawns, Color color) {
        if (color == Color::WHITE) {
            return ((pawns << 7) & BitboardConstants::NOT_FILE_H) |
                   ((pawns << 9) & BitboardConstants::NOT_FILE_A);
        } else {
            return ((pawns >> 7) & BitboardConstants::NOT_FILE_A) |
                   ((pawns >> 9) & BitboardConstants::NOT_FILE_H);
        }
    }
    
    // 폰의 왼쪽 공격
    static Bitboard getPawnLeftAttacks(Bitboard pawns, Color color) {
        if (color == Color::WHITE) {
            return (pawns << 7) & BitboardConstants::NOT_FILE_H;
        } else {
            return (pawns >> 9) & BitboardConstants::NOT_FILE_H;
        }
    }
    
    // 폰의 오른쪽 공격
    static Bitboard getPawnRightAttacks(Bitboard pawns, Color color) {
        if (color == Color::WHITE) {
            return (pawns << 9) & BitboardConstants::NOT_FILE_A;
        } else {
            return (pawns >> 7) & BitboardConstants::NOT_FILE_A;
        }
    }
    
    // 폰의 전진 (1칸)
    static Bitboard getPawnSinglePush(Bitboard pawns, Bitboard empty, Color color) {
        if (color == Color::WHITE) {
            return (pawns << 8) & empty;
        } else {
            return (pawns >> 8) & empty;
        }
    }
    
    // 폰의 전진 (2칸)
    static Bitboard getPawnDoublePush(Bitboard pawns, Bitboard empty, Color color) {
        Bitboard single_push = getPawnSinglePush(pawns, empty, color);
        if (color == Color::WHITE) {
            return (single_push << 8) & empty & BitboardConstants::RANK_4;
        } else {
            return (single_push >> 8) & empty & BitboardConstants::RANK_5;
        }
    }
    
    // 프로모션 랭크 확인
    static bool isPromotionRank(int rank, Color color) {
        return (color == Color::WHITE && rank == 7) || 
               (color == Color::BLACK && rank == 0);
    }
    
private:
    static constexpr Bitboard RANK_4 = 0x00000000FF000000ULL;
    static constexpr Bitboard RANK_5 = 0x000000FF00000000ULL;
};

#endif // PAWN_UTILS_H
```

### 3. 예외 처리 강화

#### Opening Book 로딩 개선
```cpp
// 현재 Book.cpp (문제점: 예외 처리 부족)
void OpeningBook::load() {
    std::ifstream file(filename);
    // 파일 열기 실패 시 처리 없음
}

// 개선된 버전
class OpeningBookException : public std::exception {
private:
    std::string message;
public:
    OpeningBookException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

void OpeningBook::load() {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw OpeningBookException("Failed to open opening book file: " + filename);
    }
    
    try {
        // 기존 로딩 로직
        loadEcoData(file);
    } catch (const std::exception& e) {
        throw OpeningBookException("Error parsing opening book: " + std::string(e.what()));
    }
}
```

#### FEN 파싱 검증 강화
```cpp
// FEN 검증 함수 추가 제안
class FenValidator {
public:
    struct ValidationResult {
        bool isValid;
        std::string errorMessage;
    };
    
    static ValidationResult validate(const std::string& fen) {
        std::vector<std::string> parts = splitFen(fen);
        
        if (parts.size() != 6) {
            return {false, "FEN must have exactly 6 parts"};
        }
        
        // 1. 보드 포지션 검증
        if (!validateBoardPosition(parts[0])) {
            return {false, "Invalid board position in FEN"};
        }
        
        // 2. 활성 색상 검증
        if (parts[1] != "w" && parts[1] != "b") {
            return {false, "Active color must be 'w' or 'b'"};
        }
        
        // 3. 캐슬링 권한 검증
        if (!validateCastlingRights(parts[2])) {
            return {false, "Invalid castling rights"};
        }
        
        // 4. 앙파상 타겟 검증
        if (!validateEnPassantTarget(parts[3])) {
            return {false, "Invalid en passant target"};
        }
        
        // 5. 하프무브 클럭 검증
        if (!isNumeric(parts[4])) {
            return {false, "Halfmove clock must be numeric"};
        }
        
        // 6. 풀무브 넘버 검증
        if (!isNumeric(parts[5])) {
            return {false, "Fullmove number must be numeric"};
        }
        
        return {true, ""};
    }
    
private:
    static bool validateBoardPosition(const std::string& board);
    static bool validateCastlingRights(const std::string& castling);
    static bool validateEnPassantTarget(const std::string& ep);
    static bool isNumeric(const std::string& str);
    static std::vector<std::string> splitFen(const std::string& fen);
};
```

### 4. 성능 최적화 - 기동성 평가 개선

#### 현재 문제점
```cpp
// AIPlayer.cpp - 비효율적인 기동성 계산
int AIPlayer::evaluateMobility(const BoardState& boardState) const {
    // 모든 합법적 수를 생성 (비용이 높음)
    std::vector<Move> white_moves, black_moves;
    // ... 전체 수 생성 로직
}
```

#### 개선된 기동성 평가
```cpp
// 새로운 MobilityEvaluator 클래스
class MobilityEvaluator {
public:
    static int evaluateMobility(const BoardState& boardState) {
        int mobility_score = 0;
        
        // 각 색상별로 기동성 계산
        for (int color_idx = 0; color_idx < 2; ++color_idx) {
            int sign = (color_idx == 0) ? 1 : -1;
            Color color = (color_idx == 0) ? Color::WHITE : Color::BLACK;
            
            mobility_score += calculatePieceMobility(boardState, color) * sign;
        }
        
        return mobility_score;
    }
    
private:
    static int calculatePieceMobility(const BoardState& boardState, Color color) {
        int color_idx = (color == Color::WHITE) ? 0 : 1;
        int mobility = 0;
        
        // 나이트 기동성 (빠른 계산)
        Bitboard knights = boardState.knight[color_idx];
        mobility += countKnightMobility(knights, boardState.all_pieces) * KNIGHT_MOBILITY_WEIGHT;
        
        // 비숍 기동성 (Magic Bitboards 활용)
        Bitboard bishops = boardState.bishop[color_idx];
        mobility += countBishopMobility(bishops, boardState.all_pieces) * BISHOP_MOBILITY_WEIGHT;
        
        // 룩 기동성
        Bitboard rooks = boardState.rook[color_idx];
        mobility += countRookMobility(rooks, boardState.all_pieces) * ROOK_MOBILITY_WEIGHT;
        
        // 퀸 기동성
        Bitboard queens = boardState.queen[color_idx];
        mobility += countQueenMobility(queens, boardState.all_pieces) * QUEEN_MOBILITY_WEIGHT;
        
        return mobility;
    }
    
    // 실제 수를 생성하지 않고 공격 칸의 개수만 계산
    static int countKnightMobility(Bitboard knights, Bitboard all_pieces) {
        int mobility = 0;
        while (knights) {
            int square = __builtin_ctzll(knights);
            Bitboard attacks = getKnightAttacks(square);
            mobility += __builtin_popcountll(attacks & ~all_pieces); // 빈 칸으로의 이동만
            knights &= knights - 1; // clear LSB
        }
        return mobility;
    }
    
    static Bitboard getKnightAttacks(int square);
    static int countBishopMobility(Bitboard bishops, Bitboard all_pieces);
    static int countRookMobility(Bitboard rooks, Bitboard all_pieces);
    static int countQueenMobility(Bitboard queens, Bitboard all_pieces);
    
    static constexpr int KNIGHT_MOBILITY_WEIGHT = 4;
    static constexpr int BISHOP_MOBILITY_WEIGHT = 5;
    static constexpr int ROOK_MOBILITY_WEIGHT = 2;
    static constexpr int QUEEN_MOBILITY_WEIGHT = 1;
};
```

### 5. UCI 프로토콜 개선 - 시간 제어

#### 현재 문제점
```cpp
// Engine.cpp - 시간 제어 미구현
void Engine::handleGo(const std::string& line) {
    int depth = 5; // 하드코딩된 깊이만 지원
    // TODO: Parse other go options like wtime, btime, etc.
}
```

#### 개선된 시간 제어 구현
```cpp
// 새로운 TimeManager 클래스
class TimeManager {
public:
    struct TimeControl {
        int wtime = 0;          // 백의 남은 시간 (ms)
        int btime = 0;          // 흑의 남은 시간 (ms)
        int winc = 0;           // 백의 증가 시간 (ms)
        int binc = 0;           // 흑의 증가 시간 (ms)
        int movestogo = 0;      // 다음 시간 추가까지 남은 수
        int depth = 0;          // 고정 깊이 (0이면 시간 기반)
        int movetime = 0;       // 고정 시간 (ms)
        bool infinite = false;  // 무한 탐색
    };
    
    TimeManager(const TimeControl& tc, Color sideToMove) 
        : timeControl(tc), searchingSide(sideToMove) {
        calculateOptimalTime();
    }
    
    bool shouldStop() const {
        if (timeControl.infinite) return false;
        if (timeControl.depth > 0) return false; // 깊이 기반은 별도 처리
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime
        ).count();
        
        return elapsed >= optimalTime;
    }
    
    void startTiming() {
        startTime = std::chrono::steady_clock::now();
    }
    
    int getOptimalTime() const { return optimalTime; }
    
private:
    void calculateOptimalTime() {
        if (timeControl.movetime > 0) {
            optimalTime = timeControl.movetime;
            return;
        }
        
        int myTime = (searchingSide == Color::WHITE) ? timeControl.wtime : timeControl.btime;
        int myInc = (searchingSide == Color::WHITE) ? timeControl.winc : timeControl.binc;
        
        if (timeControl.movestogo > 0) {
            // 정확한 수 기반 계산
            optimalTime = (myTime / timeControl.movestogo) + myInc - SAFETY_MARGIN;
        } else {
            // 갑작스런 죽음 또는 증가시간 기반
            optimalTime = myTime / 30 + myInc - SAFETY_MARGIN; // 대략 30수 예상
        }
        
        optimalTime = std::max(optimalTime, MIN_THINK_TIME);
        optimalTime = std::min(optimalTime, myTime / 2); // 시간의 절반 이상 사용 금지
    }
    
    TimeControl timeControl;
    Color searchingSide;
    std::chrono::steady_clock::time_point startTime;
    int optimalTime;
    
    static constexpr int SAFETY_MARGIN = 50;    // 50ms 안전 마진
    static constexpr int MIN_THINK_TIME = 100;  // 최소 사고 시간
};

// Engine.cpp 개선
void Engine::handleGo(const std::string& line) {
    if (searching) return;
    
    TimeManager::TimeControl tc = parseTimeControl(line);
    timeManager = std::make_unique<TimeManager>(tc, game.getBoard().getState().currentTurn);
    
    if (tc.depth > 0) {
        searchBestMoveWithDepth(tc.depth);
    } else {
        searchBestMoveWithTime();
    }
}

TimeManager::TimeControl Engine::parseTimeControl(const std::string& line) {
    TimeManager::TimeControl tc;
    std::istringstream iss(line);
    std::string token;
    iss >> token; // consume "go"
    
    while (iss >> token) {
        if (token == "wtime") iss >> tc.wtime;
        else if (token == "btime") iss >> tc.btime;
        else if (token == "winc") iss >> tc.winc;
        else if (token == "binc") iss >> tc.binc;
        else if (token == "movestogo") iss >> tc.movestogo;
        else if (token == "depth") iss >> tc.depth;
        else if (token == "movetime") iss >> tc.movetime;
        else if (token == "infinite") tc.infinite = true;
    }
    
    return tc;
}
```

### 6. Late Move Reductions (LMR) 구현

```cpp
// AIPlayer.cpp에 LMR 추가
int AIPlayer::minimax(Game& game, int depth, int ply, bool maximizingPlayer, 
                     int alpha, int beta) {
    // ... 기존 코드 ...
    
    // Move ordering 후 탐색 시작
    int moveCount = 0;
    for (const auto& scoredMove : scoredMoves) {
        moveCount++;
        const Move& move = scoredMove.move;
        
        game.makeMove(move);
        int score;
        
        // Late Move Reductions 적용
        bool needsFullSearch = true;
        
        if (depth > 2 && moveCount > 3 && !inCheck && 
            !isCapture(move) && !isPromotion(move) && 
            !isTactical(move)) {
            
            // Reduced depth search
            int reduction = calculateReduction(depth, moveCount);
            score = -minimax(game, depth - reduction - 1, ply + 1, 
                           !maximizingPlayer, -alpha - 1, -alpha);
            
            // PVS null window search가 실패한 경우만 full search
            needsFullSearch = (score > alpha);
        }
        
        if (needsFullSearch) {
            if (moveCount == 1) {
                // First move: full window
                score = -minimax(game, depth - 1, ply + 1, 
                               !maximizingPlayer, -beta, -alpha);
            } else {
                // PVS: null window search first
                score = -minimax(game, depth - 1, ply + 1, 
                               !maximizingPlayer, -alpha - 1, -alpha);
                
                if (score > alpha && score < beta) {
                    // Re-search with full window
                    score = -minimax(game, depth - 1, ply + 1, 
                                   !maximizingPlayer, -beta, -alpha);
                }
            }
        }
        
        game.undoMove(); // undo 기능 필요
        
        // ... 나머지 알파베타 로직
    }
    
    return alpha;
}

int AIPlayer::calculateReduction(int depth, int moveCount) {
    // 기본 reduction: 1
    int reduction = 1;
    
    // 깊이가 깊을수록 더 많이 reduction
    if (depth > 6) reduction++;
    
    // 후반 수일수록 더 많이 reduction
    if (moveCount > 6) reduction++;
    if (moveCount > 12) reduction++;
    
    // 최대 reduction 제한
    return std::min(reduction, depth - 1);
}

bool AIPlayer::isTactical(const Move& move) const {
    // 전술적 수인지 판단 (체크, 캡처, 프로모션 등)
    return isCapture(move) || isPromotion(move) || givesCheck(move);
}
```

이러한 개선사항들을 단계적으로 적용하면 코드 품질과 성능이 크게 향상될 것입니다.