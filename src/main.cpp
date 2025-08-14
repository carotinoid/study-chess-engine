#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <locale>
#include <chrono>
#include "Game.h"
#include "AIPlayer.h"
#include "MagicBitboards.h"



std::string squareToString(Square s) {
    return std::string(1, (char)('a' + s.file)) + std::string(1, (char)('1' + s.rank));
}

std::optional<Move> parseMove(const std::string& input) {
    if (input.length() < 4 || input.length() > 5) return std::nullopt;
    Square start, end;
    start.file = tolower(input[0]) - 'a'; start.rank = input[1] - '1';
    end.file = tolower(input[2]) - 'a'; end.rank = input[3] - '1';
    if (!start.isValid() || !end.isValid()) return std::nullopt;
    Move m{start, end};
    if (input.length() == 5) {
        switch (tolower(input[4])) {
            case 'q': m.promotionPiece = PieceType::QUEEN; break;
            case 'r': m.promotionPiece = PieceType::ROOK; break;
            case 'b': m.promotionPiece = PieceType::BISHOP; break;
            case 'n': m.promotionPiece = PieceType::KNIGHT; break;
            default: return std::nullopt;
        }
    }
    return m;
}

void renderBoard(const Game& game) {
    std::wcout.imbue(std::locale(""));
    const BitboardRepresentation& board = game.getBoard();
    const BoardState& boardState = board.getState();

    auto getPieceAt = [&](int rank, int file) -> std::optional<std::pair<PieceType, Color>> {
        Bitboard mask = 1ULL << (rank * 8 + file);
        if (boardState.pawn[0] & mask) return {{PieceType::PAWN, Color::WHITE}};
        if (boardState.pawn[1] & mask) return {{PieceType::PAWN, Color::BLACK}};
        if (boardState.knight[0] & mask) return {{PieceType::KNIGHT, Color::WHITE}};
        if (boardState.knight[1] & mask) return {{PieceType::KNIGHT, Color::BLACK}};
        if (boardState.bishop[0] & mask) return {{PieceType::BISHOP, Color::WHITE}};
        if (boardState.bishop[1] & mask) return {{PieceType::BISHOP, Color::BLACK}};
        if (boardState.rook[0] & mask) return {{PieceType::ROOK, Color::WHITE}};
        if (boardState.rook[1] & mask) return {{PieceType::ROOK, Color::BLACK}};
        if (boardState.queen[0] & mask) return {{PieceType::QUEEN, Color::WHITE}};
        if (boardState.queen[1] & mask) return {{PieceType::QUEEN, Color::BLACK}};
        if (boardState.king[0] & mask) return {{PieceType::KING, Color::WHITE}};
        if (boardState.king[1] & mask) return {{PieceType::KING, Color::BLACK}};
        return std::nullopt;
    };

    auto getUnicodeChar = [](PieceType type, Color color) -> wchar_t {
        switch (type) {
            case PieceType::PAWN:   return (color == Color::WHITE) ? L'♙' : L'♟';
            case PieceType::KNIGHT: return (color == Color::WHITE) ? L'♘' : L'♞';
            case PieceType::BISHOP: return (color == Color::WHITE) ? L'♗' : L'♝';
            case PieceType::ROOK:   return (color == Color::WHITE) ? L'♖' : L'♜';
            case PieceType::QUEEN:  return (color == Color::WHITE) ? L'♕' : L'♛';
            case PieceType::KING:   return (color == Color::WHITE) ? L'♔' : L'♚';
        }
        return L' ';
    };

    std::wcout << L"\n   a  b  c  d  e  f  g  h\n";
    std::wcout << L" +------------------------+\n";
    for (int r = 7; r >= 0; --r) {
        std::wcout << r + 1 << L"|";
        for (int f = 0; f < 8; ++f) {
            auto piece = getPieceAt(r, f);
            bool isLightSquare = (r + f) % 2 != 0;
            if (piece) {
                std::wcout << L' ' << getUnicodeChar(piece->first, piece->second) << L' ';
            } else {
                std::wcout << (isLightSquare ? L"   " : L"███");
            }
        }
        std::wcout << L"|" << r + 1 << L"\n";
    }
    std::wcout << L" +------------------------+\n";
    std::wcout << L"   a  b  c  d  e  f  g  h\n\n";
}

int main() {
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale(""));
    std::wcin.imbue(std::locale(""));

    MagicBitboards::init();

    Game game;
    AIPlayer ai;
    std::string input;
    bool vsAI = false;
    Color playerColor = Color::WHITE;
    int ai_difficulty = 3;

    std::wcout << L"체스 게임에 오신 것을 환영합니다!" << std::endl;
    std::wcout << L"1. 사람과 대결" << std::endl;
    std::wcout << L"2. AI와 대결" << std::endl;
    std::wcout << L"모드를 선택하세요: ";
    while (true) {
        std::cin >> input;
        if (input == "1" || input == "2") break;
        std::wcout << L"잘못된 입력입니다. 1 또는 2를 입력하세요: ";
    }

    if (input == "2") {
        vsAI = true;
        std::wcout << L"백(w)과 흑(b) 중 선택하세요: ";
        while (true) {
            std::cin >> input;
            if (input.length() == 1 && (tolower(input[0]) == 'w' || tolower(input[0]) == 'b')) break;
            std::wcout << L"잘못된 입력입니다. w 또는 b를 입력하세요: ";
        }
        if (tolower(input[0]) == 'b') {
            playerColor = Color::BLACK;
        }

        std::wcout << L"AI 난이도를 설정하세요 (1-10, 높을수록 강함): ";
        while (!(std::cin >> ai_difficulty) || ai_difficulty < 1 || ai_difficulty > 10) {
            std::wcout << L"잘못된 입력입니다. 1에서 10 사이의 숫자를 입력하세요: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    while (game.getStatus() == GameStatus::ONGOING) {
        renderBoard(game);
        Color currentTurn = game.getBoard().getState().currentTurn;
        std::wcout << (currentTurn == Color::WHITE ? L"백" : L"흑") << L"의 차례입니다. " << std::endl;
        if (game.getBoard().isKingInCheck(currentTurn)) {
           std::wcout << L"체크 상태입니다!" << std::endl;
        }

        if (vsAI && currentTurn != playerColor) {
            std::wcout << L"AI가 생각 중입니다..." << std::endl;
            auto start_time = std::chrono::high_resolution_clock::now();
            Move aiMove = ai.findBestMove(game, ai_difficulty);
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = end_time - start_time;
            
            std::wcout << L"AI가 " << squareToString(aiMove.start).c_str() << squareToString(aiMove.end).c_str()
                       << L" 수를 두었습니다. (" << elapsed.count() << L"초 소요)" << std::endl;
            game.makeMove(aiMove);
        } else {
            std::wcout << L"수를 입력하세요 (예: e2e4, e7e8q): ";
            std::cin >> input;
            if (input == "exit" || input == "quit") break;

            std::optional<Move> move = parseMove(input);
            if (!move) {
                std::wcout << L"잘못된 형식입니다." << std::endl;
                continue;
            }
            
            std::vector<Move> legalMoves = game.generateAllLegalMoves();
            std::wcout << L"Possible moves: ";
            for (const auto& m : legalMoves) {
                std::wcout << m.toString().c_str() << L" ";
            }
            std::wcout << std::endl;
            bool moveIsLegal = false;
            for(const auto& legalMove : legalMoves) {
                if (legalMove == *move) {
                    moveIsLegal = true;
                    break;
                }
            }

            if (moveIsLegal) {
                game.makeMove(*move);
            } else {
                std::wcout << L"둘 수 없는 수입니다." << std::endl;
            }
        }
    }

    renderBoard(game);
    std::wcout << L"게임 종료!" << std::endl;
    GameStatus finalStatus = game.getStatus();
    
    if (finalStatus == GameStatus::CHECKMATE) {
        std::wcout << ((game.getBoard().getState().currentTurn == Color::WHITE) ? L"흑" : L"백") << L"의 승리 (체크메이트)!" << std::endl;
    } else if (finalStatus == GameStatus::STALEMATE) {
        std::wcout << L"무승부 (스테일메이트)!" << std::endl;
    } else {
        std::wcout << L"무승부!" << std::endl;
    }

    return 0;
}
