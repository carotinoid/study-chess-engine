# C++ 체스 프로젝트 개발 문서

이 문서는 터미널 기반 C++ 체스 게임의 개발 현황, 향후 목표, 그리고 주요 가이드라인을 통합하여 관리합니다.

## 1. 프로젝트 개요

이 프로젝트의 목표는 최첨단 AI 체스 엔진을 개발하는 것입니다. 최종적으로는 AI 간의 대결을 통해 엔진의 성능을 지속적으로 테스트하고 개선하며, UCI 프로토콜을 완벽하게 지원하여 범용 체스 GUI와 연동되는 고성능 엔진으로 완성하는 것을 목표로 합니다.

---

## 2. 현재 구현 현황 (Implemented)

### 핵심 엔진
-   **보드 표현**: 비트보드(Bitboard)를 사용한 효율적인 게임 상태 관리.
-   **게임 로직**: 캐슬링, 앙파상, 폰 프로모션을 포함한 모든 체스 규칙 구현 완료.
-   **게임 종료 판정**: 체크메이트, 스테일메이트, 50수 규칙, 기물 부족 무승부 등 정확한 게임 종료 판정 로직 구현.
-   **공격 경로 계산**: **매직 비트보드(Magic Bitboards)**를 구현하여 슬라이딩 기물(룩, 비숍, 퀸)의 공격 경로를 매우 빠르게 계산.

### AI 플레이어
-   **탐색 알고리즘**:
    -   알파-베타 가지치기를 적용한 미니맥스(Minimax) 탐색.
    -   **치환 테이블 (Transposition Table)**: Zobrist Hashing을 이용해 이미 탐색한 포지션의 평가 결과를 저장하고 재사용하여, 중복 계산을 제거하고 탐색 속도를 획기적으로 향상.
    -   **반복적 심화(Iterative Deepening)**를 도입하여 제한 시간 내 최적의 수를 찾고 탐색 효율을 증대.
    -   **수 정렬 (Move Ordering)**: MVV-LVA(Most Valuable Victim - Least Valuable Attacker) 기법을 도입하여 유망한 수를 우선적으로 탐색하고 알파-베타 가지치기 효율을 극대화.
    -   **정적 탐색 (Quiescence Search)**: 탐색 깊이의 한계점에서 발생하는 '수평선 효과'를 방지하기 위해, 캡처와 같이 전술적으로 불안정한 상황에서는 탐색을 추가로 진행하여 안정된 국면까지 분석하도록 개선.
-   **평가 함수**:
    -   기물 가치 및 위치 점수표(PST) 기반의 기본 평가.
    -   **테이퍼드 평가 (Tapered Evaluation)**: 게임 단계를 오프닝/미들게임과 엔드게임으로 구분하여, 단계에 맞게 PST를 동적으로 조절.
    -   기물 기동성, 중앙 장악, 폰 구조, 킹 안전성 등 다양한 요소 평가.
    -   **전략적 요소 추가**: 쌍비숍(Bishop Pair) 보너스, 열린 파일의 룩(Rook on Open File) 보너스 등 전략적 개념 추가.
    -   **정적 위협 분석**: 상대에게 공격받는 아군 기물에 페널티를, 아군이 공격하는 상대 기물에 보너스를 부여하여 단기적인 전술적 판단력 향상.

### Web UI
-   **Flask 기반 인터페이스**: Python Flask를 사용하여 엔진을 제어하는 웹 UI 제공.
-   **탐색 깊이 설정**: 사용자가 Web UI에서 직접 AI의 탐색 깊이(depth)를 조절 가능.
-   **엔진 상태 표시**: 엔진이 수를 계산 중일 때(Thinking), 응답을 기다릴 때(Ready), 오류가 발생했을 때(Error) 등 현재 상태를 UI에 표시.

---

## 3. 향후 개발 목표 (To-Do)

### 0순위: 긴급 버그 수정
-   [ ] **test make 통과하기**

### 1순위: UCI 프로토콜 및 엔진 프레임워크
-   [ ] **UCI 프로토콜 구현**: Arena, ChessBase와 같은 표준 체스 GUI와 엔진을 연동할 수 있도록 지원. 엔진의 성능을 측정하고 다른 엔진과 대결하기 위한 필수 기능. [진행중]
-   [ ] **AI 대결 프레임워크**: AI 간의 대결을 자동화하여, 평가 함수 및 탐색 알고리즘 변경에 따른 성능 변화를 통계적으로 측정하고 분석하는 시스템을 구축.

### 2순위: 탐색 알고리즘 고도화
-   [ ] **수 정렬 (Move Ordering) 개선**: Killer Moves, History Heuristic 등 더 정교한 수 정렬 기법을 도입하여 탐색 효율을 추가로 개선.
-   [ ] **Null Move Pruning**: 상대방이 한 수를 쉬게 하고 탐색을 진행하여, 위협적이지 않은 상황에서 탐색 깊이를 줄이는 가지치기 기법.
-   [ ] **Late Move Reductions (LMR)**: 탐색 후반부에 있는 수들의 탐색 깊이를 줄여, 유망하지 않은 브랜치를 빠르게 잘라내는 기법.

### 3순위: 평가 함수 정교화
-   [ ] **전략적 요소 추가**: 나이트 전초기지(Knight Outpost), 핀(Pinned)에 걸린 기물, 연결된 룩(Connected Rooks) 등 더 복잡하고 전략적인 개념들을 평가 함수에 추가.
-   [ ] **오프닝 북 (Opening Book)**: 전문가의 오프닝 라인을 데이터베이스로 구축하여, 초반 수를 계산 없이 즉시 처리해 성능과 안정성을 확보.

---

## 4. Git 커밋 가이드

커밋 메시지 작성 시, 아래 태그를 메시지 앞에 붙여 변경 사항을 명확히 분류합니다.

-   `feat:` 새로운 기능 추가
-   `fix:` 버그 수정
-   `refactor:` 기능 변경 없는 코드 구조 개선
-   `docs:` 문서 수정
-   `style:` 코드 포맷팅, 세미콜론 누락 등 스타일 관련 수정
-   `test:` 테스트 코드 추가 또는 수정
-   `chore:` 빌드 프로세스, 라이브러리 등 기타 변경사항

**예시**: `feat: 반복적 심화 탐색 기능 추가`

---

## 5. 코딩 컨벤션 (Coding Convention)

코드의 일관성, 가독성, 유지보수성을 높이기 위해 다음과 같은 코딩 컨벤션을 정의하고 준수합니다.

-   **이름 규칙 (Naming Convention)**:
    -   **클래스, 구조체, 네임스페이스, 열거형**: `PascalCase` (예: `Game`, `Move`, `Bitboard`)
    -   **공개(Public) 함수/메서드**: `PascalCase` (예: `MakeMove`, `GenerateMoves`)
    -   **비공개(Private) 함수/메서드**: `camelCase` (예: `generatePawnMoves`, `minimax`)
    -   **변수 (멤버 및 지역)**: `camelCase` (예: `sideToMove`, `searchDepth`)
    -   **상수, 열거형 멤버**: `ALL_CAPS_SNAKE_CASE` (예: `WHITE`, `PAWN`)
-   **포맷팅 (Formatting)**:
    -   **들여쓰기**: 공백 4칸(4 spaces)을 사용합니다.
    -   **괄호 스타일**: Allman 스타일을 따릅니다. 여는 중괄호(`{`)는 항상 새 줄에 위치합니다.
    -   **포인터/참조**: 타입명 바로 뒤에 붙입니다. (예: `Board& board`, `const std::string& fen`)

---

## 6. 주요 공개 API (Public API)

이 프로젝트의 핵심 기능을 노출하는 주요 공개 함수 및 메서드 목록입니다.

### `BitboardRepresentation` (in `Bitboard.h`)
-   `BitboardRepresentation()`: `BitboardRepresentation` 객체를 생성합니다.
-   `void setupInitialPosition()`: 체스 보드를 초기 상태로 설정합니다.
-   `const BoardState& getState() const`: 현재 보드의 상태(Bitboard, 턴, 캐슬링 권한 등)를 담은 `BoardState` 객체를 반환합니다.
-   `void makeMove(const Move& move)`: 주어진 수를 실행하여 보드 상태를 업데이트합니다.
-   `bool isKingInCheck(Color kingColor) const`: 특정 색상의 킹이 체크 상태인지 확인합니다.

### `BitboardUtils` (in `Bitboard.h`)
-   `void printBitboard(Bitboard b)`: 주어진 비트보드를 8x8 체스판 형태로 터미널에 출력합니다.
-   `bool isSquareAttackedBy(Square s, Color attackerColor, const BoardState& boardState)`: 특정 칸이 상대방에게 공격받고 있는지 확인합니다.

### `MagicBitboards` (in `MagicBitboards.h`)
-   `void Init()`: 매직 비트보드 관련 데이터(공격 테이블 등)를 초기화합니다.
-   `Bitboard get_rook_attacks(Square s, Bitboard occupancy)`: 특정 위치의 룩에 대한 공격 비트보드를 반환합니다.
-   `Bitboard get_bishop_attacks(Square s, Bitboard occupancy)`: 특정 위치의 비숍에 대한 공격 비트보드를 반환합니다.
-   `Bitboard get_queen_attacks(Square s, Bitboard occupancy)`: 특정 위치의 퀸에 대한 공격 비트보드를 반환합니다.

### `MoveGen` (in `MoveGen.h`)
-   `void generateMoves(const BoardState& boardState, std::vector<Move>& moves)`: 모든 의사-합법적인 수를 생성합니다.
-   `void generateCaptureMoves(const BoardState& boardState, std::vector<Move>& moves)`: 모든 의사-합법적인 캡처 수를 생성합니다.
-   `void generatePawnMoves(const BoardState& boardState, std::vector<Move>& moves)`: 폰의 모든 의사-합법적인 수를 생성합니다.
-   `void generateKnightMoves(const BoardState& boardState, std::vector<Move>& moves)`: 나이트의 모든 의사-합법적인 수를 생성합니다.
-   `void generateBishopMoves(const BoardState& boardState, std::vector<Move>& moves)`: 비숍의 모든 의사-합법적인 수를 생성합니다.
-   `void generateRookMoves(const BoardState& boardState, std::vector<Move>& moves)`: 룩의 모든 의사-합법적인 수를 생성합니다.
-   `void generateQueenMoves(const BoardState& boardState, std::vector<Move>& moves)`: 퀸의 모든 의사-합법적인 수를 생성합니다.
-   `void generateKingMoves(const BoardState& boardState, std::vector<Move>& moves)`: 킹의 모든 의사-합법적인 수를 생성합니다.

### `Game` (in `Game.h`)
-   `Game()`: 새로운 체스 게임 객체를 생성합니다.
-   `BitboardRepresentation& getBoard()`: 현재 게임의 보드 객체에 대한 참조를 반환합니다.
-   `const BitboardRepresentation& getBoard() const`: 현재 게임의 보드 객체에 대한 상수 참조를 반환합니다.
-   `GameStatus getStatus() const`: 현재 게임의 상태(진행중, 체크메이트 등)를 반환합니다.
-   `std::vector<Move> getLegalMovesForPieceAt(Square s)`: 특정 위치의 기물에 대한 모든 합법적인 수를 반환합니다.
-   `bool makeMove(const Move& move)`: 주어진 수를 실행하고 보드 상태를 업데이트합니다. 성공 시 `true`를 반환합니다.
-   `std::vector<Move> generateAllLegalMoves()`: 현재 상태에서 가능한 모든 합법적인 수를 반환합니다.

### `AIPlayer` (in `AIPlayer.h`)
-   `Move findBestMove(Game& game, int depth)`: 주어진 게임 상태에서 AI가 지정된 깊이까지 탐색하여 찾아낸 최선의 수를 반환합니다.
