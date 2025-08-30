# 즉시 수정 가능한 개선사항

## 🚨 발견된 구체적 문제점 및 해결책

### 1. Engine.cpp의 중복 코드 문제

#### 문제점
```cpp
// Engine.cpp 라인 76-91에서 moves 파싱이 중복됨
if (token == "moves") {
    while (iss >> token) {
        Move move = parseMove(token, game);
        std::cout << move << std::endl;  // 디버그 출력이 남아있음
        game.makeMove(move);
    }
}

iss >> token;
if (token == "moves") {  // 동일한 로직이 중복
    while (iss >> token) {
        Move move = parseMove(token, game);
        std::cout << move << std::endl;  // 디버그 출력이 남아있음
        game.makeMove(move);
    }
}
```

#### 수정 방안
```cpp
// 중복 제거 및 디버그 출력 제거
void Engine::handlePosition(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // consume "position"

    iss >> token;
    if (token == "startpos") {
        game = Game();
    } else if (token == "fen") {
        std::string fen;
        for (int i = 0; i < 6; ++i) {
            iss >> token;
            fen += token + (i == 5 ? "" : " ");
        }
        game = Game(fen);
    }

    // moves 처리 (한 번만)
    iss >> token;
    if (token == "moves") {
        while (iss >> token) {
            try {
                Move move = parseMove(token, game);
                if (!game.makeMove(move)) {
                    std::cerr << "Error: Invalid move " << token << std::endl;
                    break;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing move " << token << ": " << e.what() << std::endl;
                break;
            }
        }
    }
}
```

### 2. AIPlayer.cpp의 매직 넘버 문제

#### 문제점
```cpp
// PST에서 하드코딩된 배열 인덱스
int sq_idx = color_idx == 0 ? i : 63 - i;  // 63은 매직 넘버
```

#### 수정 방안
```cpp
// 상수로 정의
namespace BoardConstants {
    constexpr int BOARD_SIZE = 64;
    constexpr int LAST_SQUARE = BOARD_SIZE - 1;
    constexpr int FILES = 8;
    constexpr int RANKS = 8;
}

// 사용
int sq_idx = color_idx == 0 ? i : BoardConstants::LAST_SQUARE - i;
```

### 3. MoveGen.cpp의 프로모션 로직 중복

#### 문제점
프로모션 생성 코드가 여러 곳에서 반복됨

#### 수정 방안
```cpp
// 새로운 유틸리티 함수
namespace MoveGenUtils {
    void addPromotionMoves(std::vector<Move>& moves, Square from, Square to) {
        moves.push_back({from, to, PieceType::QUEEN});
        moves.push_back({from, to, PieceType::ROOK});
        moves.push_back({from, to, PieceType::BISHOP});
        moves.push_back({from, to, PieceType::KNIGHT});
    }
    
    void addMoveOrPromotions(std::vector<Move>& moves, Square from, Square to, Color color) {
        bool isPromotionRank = (color == Color::WHITE && to.rank == 7) || 
                              (color == Color::BLACK && to.rank == 0);
        
        if (isPromotionRank) {
            addPromotionMoves(moves, from, to);
        } else {
            moves.push_back({from, to});
        }
    }
}
```

### 4. 메모리 효율성 개선

#### 문제점
불필요한 벡터 복사가 자주 발생함

#### 수정 방안
```cpp
// 현재: 값으로 반환 (복사 발생)
std::vector<Move> Game::generateAllLegalMoves() const {
    std::vector<Move> moves;
    // ...
    return moves;  // 복사 발생
}

// 개선: 참조로 전달
void Game::generateAllLegalMoves(std::vector<Move>& moves) const {
    moves.clear();
    moves.reserve(64);  // 메모리 미리 할당
    // ...
}

// 또는 move semantics 활용
std::vector<Move> Game::generateAllLegalMoves() const {
    std::vector<Move> moves;
    moves.reserve(64);
    // ...
    return std::move(moves);  // move semantics
}
```

### 5. 타입 안전성 개선

#### 문제점
```cpp
// 배열 인덱스로 Color를 직접 사용
int color_idx = (boardState.currentTurn == Color::WHITE) ? 0 : 1;
```

#### 수정 방안
```cpp
// 타입 안전한 변환 함수
constexpr int colorToIndex(Color color) {
    return static_cast<int>(color);
}

constexpr Color indexToColor(int index) {
    return static_cast<Color>(index);
}

// 사용
int color_idx = colorToIndex(boardState.currentTurn);
```

### 6. 에러 처리 개선

#### 문제점
많은 함수에서 에러 상황을 무시함

#### 수정 방안
```cpp
// 현재
void BitboardRepresentation::setupPositionFromFen(const std::string& fen) {
    // FEN 파싱 오류 시 아무 처리 없음
}

// 개선
enum class FenParseResult {
    SUCCESS,
    INVALID_FORMAT,
    INVALID_PIECE_PLACEMENT,
    INVALID_ACTIVE_COLOR,
    INVALID_CASTLING_RIGHTS,
    INVALID_EN_PASSANT,
    INVALID_MOVE_COUNTERS
};

FenParseResult BitboardRepresentation::setupPositionFromFen(const std::string& fen) {
    try {
        // FEN 파싱 로직
        return FenParseResult::SUCCESS;
    } catch (const std::exception& e) {
        return FenParseResult::INVALID_FORMAT;
    }
}
```

### 7. 컴파일러 최적화 힌트 추가

#### 개선 방안
```cpp
// 자주 호출되는 함수에 inline 추가
inline Bitboard BitboardUtils::squareToBitboard(Square s) {
    return 1ULL << (s.rank * 8 + s.file);
}

// 분기 예측 힌트 추가 (GCC/Clang)
#ifdef __GNUC__
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

// 사용 예시
if (LIKELY(move_is_legal)) {
    // 일반적인 경우
} else {
    // 드문 경우
}
```

### 8. 상수 표현식 최적화

#### 문제점
런타임에 계산되는 상수들

#### 수정 방안
```cpp
// 현재
const int MATE_SCORE = 32000;

// 개선: constexpr 사용
constexpr int MATE_SCORE = 32000;
constexpr int MAX_PLY = 64;
constexpr int INFINITY_SCORE = MATE_SCORE + MAX_PLY;

// 더 나아가 템플릿 메타프로그래밍
template<int N>
constexpr uint64_t file_mask() {
    static_assert(N >= 0 && N < 8, "File index out of range");
    return 0x0101010101010101ULL << N;
}

constexpr uint64_t FILE_A_MASK = file_mask<0>();
constexpr uint64_t FILE_H_MASK = file_mask<7>();
```

### 9. 스마트 포인터 사용 개선

#### 현재 코드
```cpp
// AIPlayer.cpp에서 raw pointer나 unique_ptr 혼용
std::unique_ptr<TranspositionTable> transpositionTable;
std::unique_ptr<OpeningBook> openingBook;
```

#### 개선 방안
```cpp
// RAII 및 exception safety 보장
class AIPlayer {
private:
    // 복사 불가능한 리소스들
    std::unique_ptr<TranspositionTable> transpositionTable_;
    std::unique_ptr<OpeningBook> openingBook_;
    
public:
    AIPlayer() 
        : transpositionTable_(std::make_unique<TranspositionTable>(64))
        , openingBook_(std::make_unique<OpeningBook>("opening/eco")) {
        
        try {
            openingBook_->load();
        } catch (const std::exception& e) {
            // 오프닝 북 로딩 실패해도 계속 동작
            std::cerr << "Warning: Failed to load opening book: " << e.what() << std::endl;
            openingBook_.reset();
        }
    }
    
    // 복사 생성자 및 대입 연산자 명시적 삭제
    AIPlayer(const AIPlayer&) = delete;
    AIPlayer& operator=(const AIPlayer&) = delete;
    
    // 이동 생성자 및 대입 연산자는 기본 구현 사용
    AIPlayer(AIPlayer&&) = default;
    AIPlayer& operator=(AIPlayer&&) = default;
};
```

### 10. 디버그 빌드 최적화

#### Makefile 개선
```makefile
# 현재 Makefile에 디버그 모드 추가
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS = -std=c++17 -Wall -g -O0 -DDEBUG -Iinclude
else
    CXXFLAGS = -std=c++17 -Wall -O3 -DNDEBUG -Iinclude
endif

# 추가 플래그
CXXFLAGS += -march=native -flto -ffast-math

# 디버그 타겟 추가
debug: DEBUG=1
debug: $(TARGET)

.PHONY: all clean test bench debug
```

이러한 개선사항들은 대부분 기존 기능을 변경하지 않으면서도 코드 품질, 안정성, 성능을 향상시킬 수 있는 것들입니다.