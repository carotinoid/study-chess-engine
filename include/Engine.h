#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include "Game.h"
#include "AIPlayer.h"

class Engine {
public:
    Engine();
    void run();

private:
    void uciLoop();
    void handleUci();
    void handleIsReady();
    void handleUciNewGame();
    void handlePosition(const std::string& line);
    void handleGo(const std::string& line);
    void handleStop();
    void handleQuit();

    void searchBestMove(int depth);

    static Move parseMove(const std::string& moveString, const Game& game);
    static std::string moveToString(const Move& move);


    Game game;
    AIPlayer ai;
    std::atomic<bool> searching;
    std::atomic<bool> quit;
    std::thread searchThread;
};

#endif // ENGINE_H
