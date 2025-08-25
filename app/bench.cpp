#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "../include/Game.h"
#include "../include/AIPlayer.h"
#include "../include/MagicBitboards.h"
#include "../include/Zobrist.h"

int main() {
    Zobrist::Init();
    MagicBitboards::Init();

    std::vector<std::string> fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/ppp1p2p/3p2p1/4Pp2/8/2P5/PP1P1PPP/RNBQKBNR w KQkq f6 0 4",
        "rnbqkbnr/pp1p1pp1/2p1p2p/8/3N4/2P5/PP1PPPPP/RNBQKB1R w KQkq - 0 4",
        "8/p1ppp2p/1p1k2r1/8/4B3/6P1/1N3PKP/8 w - - 0 1",
        "3k4/2pp4/4p3/8/Q2R2n1/2P3P1/3P1PKP/8 w - - 0 1",
        "r1bqkbnr/pppppppp/8/8/3nP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 3",
        "r1b1kb1r/ppppnppp/2n1pq2/8/4P3/1P3N2/P1PPBPPP/RNBQ1RK1 b kq - 0 5",
        "r1bqkbnr/1ppp1ppp/p1n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4",
        "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1",
        "r1b1kbnr/pp3ppp/1qn1p3/3pP3/2pP4/2P2N2/PPB2PPP/RNBQK2R b KQkq - 1 7"
    };

    AIPlayer ai;
    ai.setUseOpeningBook(false);

    int depth = 5; // Set a default depth for the benchmark

    for (const auto& fen : fens) {
        Game game(fen);
        std::cout << "FEN: " << fen << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        Move bestMove = ai.findBestMove(game, depth);
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = end - start;

        std::cout << "Best move: " << bestMove.toString() << std::endl;
        std::cout << "Time taken: " << elapsed.count() << " ms" << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    return 0;
}
