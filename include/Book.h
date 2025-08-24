#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "Game.h"

class OpeningBook {
public:
    OpeningBook(const std::string& ecoFile);
    void load();
    std::vector<Move> findMoves(const uint64_t hash) const;

private:
    Move sanToMove(const std::string& san, Game& game);
    std::string bookFile;
    std::unordered_multimap<uint64_t, Move> book;
};

#endif // BOOK_H
