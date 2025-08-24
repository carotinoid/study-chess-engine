# UCI 프로토콜 명령어

이 문서는 현재 체스 엔진에 구현된 UCI (Universal Chess Interface) 프로토콜 명령어를 상세히 설명합니다.

---

## 1. 구현된 명령어

### `uci`
엔진에 UCI 모드를 시작하도록 지시합니다. 이 명령은 GUI가 엔진 시작 시 가장 먼저 보내는 명령이어야 합니다.

- **동작**: 엔진은 자신의 이름(`id name`)과 제작자(`id author`)를 포함한 `id` 정보를 출력하고, UCI 모드 초기화가 완료되었음을 알리는 `uciok`를 보냅니다.
- **응답 예시**:
  ```
  id name MyChessEngine
  id author Gemini
  uciok
  ```

### `isready`
엔진이 다음 명령을 받을 준비가 되었는지 확인합니다. 주로 이전 명령의 처리가 완료되었는지 동기화하기 위해 사용됩니다.

- **동작**: 엔진은 모든 준비가 완료되면 `readyok`를 보냅니다.
- **응답 예시**:
  ```
  readyok
  ```

### `ucinewgame`
새로운 게임을 시작하도록 엔진에 알립니다.

- **동작**: 엔진은 내부 게임 상태(예: 보드, 히스토리)를 초기화하고 새 게임을 준비합니다. 해시 테이블 등은 초기화하지 않을 수 있습니다.

### `position [fen <fenstring> | startpos] moves <move1> <move2> ...`
보드 상태를 설정합니다.

- **`position startpos`**: 보드를 체스 기본 시작 위치로 설정합니다.
- **`position fen <fenstring>`**: 주어진 FEN 문자열에 따라 보드 상태를 설정합니다.
- **`moves <move1> <move2> ...`**: `startpos` 또는 `fen`으로 설정된 상태 이후에, 주어진 수들을 순서대로 실행하여 보드를 최종 상태로 만듭니다. 수는 Long Algebraic Notation 형식입니다 (예: `e2e4`, `e7e5`, `g1f3`, `e1g1` for castling, `e7e8q` for promotion).

- **사용 예시**:
  ```
  # 초기 상태에서 e4, e5 실행
  position startpos moves e2e4 e7e5

  # 특정 FEN 상태에서 Nf3 실행
  position fen rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1 moves g1f3
  ```

### `go`
설정된 포지션에서 최선의 수를 탐색하도록 지시합니다.

- **지원 옵션**:
  - `depth <d>`: 지정된 깊이 `d`까지 수를 탐색합니다.

- **동작**: 탐색을 시작하고, 완료되면 `bestmove` 명령어로 최선의 수를 보냅니다.
- **응답 예시**:
  ```
  bestmove g1f3
  ```

### `stop`
엔진이 현재 진행 중인 탐색을 즉시 중단하도록 지시합니다.

- **동작**: 엔진은 탐색을 멈춥니다.

### `quit`
엔진 프로그램을 종료합니다.

- **동작**: 진행 중인 모든 작업을 중단하고 엔진 프로세스를 종료합니다.

---

## 2. 부분적으로 구현되었거나 미구현된 기능

현재 엔진은 기본적인 UCI 기능은 지원하지만, 더 고급 기능들은 아직 구현되지 않았습니다.

- **`go` 명령어의 다양한 옵션**:
  - 시간 제어 (`wtime`, `btime`, `winc`, `binc`), `movestogo`, `infinite` 등 대부분의 시간 관련 옵션이 **구현되지 않았습니다.** 현재는 `depth` 기반 탐색만 가능합니다.

- **탐색 중 정보 출력 (`info` 명령어)**:
  - 엔진이 탐색을 진행하는 동안 현재 탐색 깊이, 평가 점수, 탐색 중인 수(PV) 등을 `info` 명령어로 출력하는 기능이 **구현되지 않았습니다.** 탐색이 완전히 끝난 후에만 `bestmove`를 반환합니다.

- **`setoption` 명령어**:
  - 해시 테이블 크기나 엔진의 다른 파라미터를 설정하는 `setoption` 명령어가 **구현되지 않았습니다.**

- **`ponder` (상대방 턴에 미리 생각하기)**:
  - `ponder` 기능이 **구현되지 않았습니다.**