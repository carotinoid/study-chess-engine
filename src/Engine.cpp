#include "../include/Engine.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "../include/MagicBitboards.h"
#include "../include/Zobrist.h"

Engine::Engine() : searching(false), quit(false) {
    Zobrist::Init();
    MagicBitboards::Init();
}

void Engine::run() {
    uciLoop();
}

void Engine::uciLoop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "uci") {
            handleUci();
        } else if (token == "isready") {
            handleIsReady();
        } else if (token == "ucinewgame") {
            handleUciNewGame();
        } else if (token == "position") {
            handlePosition(line);
        } else if (token == "go") {
            handleGo(line);
        } else if (token == "stop") {
            handleStop();
        } else if (token == "quit") {
            handleQuit();
            break;
        }
        if (quit) break;
    }
}

void Engine::handleUci() {
    std::cout << "id name MyChessEngine" << std::endl;
    std::cout << "id author Gemini" << std::endl;
    // TODO: Add options here if any
    std::cout << "uciok" << std::endl;
}

void Engine::handleIsReady() {
    std::cout << "readyok" << std::endl;
}

void Engine::handleUciNewGame() {
    game = Game(); // Reset the game
}

void Engine::handlePosition(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // consume "position"

    iss >> token;
    if (token == "startpos") {
        game = Game();
    } else if (token == "fen") {
        std::string fen;
        // The FEN string consists of 6 fields.
        for (int i = 0; i < 6; ++i) {
            iss >> token;
            fen += token + (i == 5 ? "" : " ");
        }
        game = Game(fen); // Create a new game with the FEN
    } else if (token == "moves") {
        while (iss >> token) {
            Move move = parseMove(token, game);
            std::cout << move << std::endl;
            game.makeMove(move);
        }
    }

    iss >> token;
    if (token == "moves") {
        while (iss >> token) {
            Move move = parseMove(token, game);
            std::cout << move << std::endl;
            game.makeMove(move);
        }
    }
}

void Engine::handleGo(const std::string& line) {
    if (searching) {
        return; // Already searching
    }

    int depth = 5; // Default search depth

    std::istringstream iss(line);
    std::string token;
    iss >> token; // "go"

    while (iss >> token) {
        if (token == "depth") {
            iss >> depth;
        }
        // TODO: Parse other go options like wtime, btime, etc.
    }
    
    if (searchThread.joinable()) {
        searchThread.join();
    }
    searchThread = std::thread(&Engine::searchBestMove, this, depth);
}

void Engine::handleStop() {
    searching = false;
    if (searchThread.joinable()) {
        searchThread.join();
    }
}

void Engine::handleQuit() {
    quit = true;
    handleStop();
}

void Engine::searchBestMove(int depth) {
    searching = true;
    Move bestMove = ai.findBestMove(game, depth);
    if (searching) { // Check if stop was called
        std::cout << "bestmove " << moveToString(bestMove) << std::endl;
    }
    searching = false;
}

Move Engine::parseMove(const std::string& moveString, const Game& game) {
    if (moveString.length() < 4 || moveString.length() > 5) return Move{};
    Square start, end;
    start.file = tolower(moveString[0]) - 'a';
    start.rank = moveString[1] - '1';
    end.file = tolower(moveString[2]) - 'a';
    end.rank = moveString[3] - '1';
    
    Move m{start, end};

    if (moveString.length() == 5) {
        switch (tolower(moveString[4])) {
            case 'q': m.promotionPiece = PieceType::QUEEN; break;
            case 'r': m.promotionPiece = PieceType::ROOK; break;
            case 'b': m.promotionPiece = PieceType::BISHOP; break;
            case 'n': m.promotionPiece = PieceType::KNIGHT; break;
            default: break; // Should not happen in valid UCI moves
        }
    }
    
    // The move needs to be validated against the legal moves to get the correct move type (e.g. castling)
    auto legalMoves = game.generateAllLegalMoves();
    for(const auto& legalMove : legalMoves) {
        if (legalMove.start == m.start && legalMove.end == m.end) {
            if (m.promotionPiece.has_value()) {
                if (legalMove.promotionPiece == m.promotionPiece) {
                    return legalMove;
                }
            } else {
                return legalMove;
            }
        }
    }

    return m; // Should be a legal move
}

std::string Engine::moveToString(const Move& move) {
    std::string str;
    str += (char)('a' + move.start.file);
    str += (char)('1' + move.start.rank);
    str += (char)('a' + move.end.file);
    str += (char)('1' + move.end.rank);
    if (move.promotionPiece) {
        switch (*move.promotionPiece) {
            case PieceType::QUEEN: str += 'q'; break;
            case PieceType::ROOK: str += 'r'; break;
            case PieceType::BISHOP: str += 'b'; break;
            case PieceType::KNIGHT: str += 'n'; break;
            default: break;
        }
    }
    return str;
}
