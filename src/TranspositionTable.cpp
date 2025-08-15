#include "../include/TranspositionTable.h"
#include <stdexcept>

TranspositionTable::TranspositionTable(size_t sizeInMB) {
    if (sizeInMB == 0) {
        numEntries = 0;
        return;
    }
    numEntries = (sizeInMB * 1024 * 1024) / sizeof(TTEntry);
    // Ensure numEntries is a power of 2 for efficient indexing
    size_t powerOf2 = 1;
    while (powerOf2 * 2 <= numEntries) {
        powerOf2 *= 2;
    }
    numEntries = powerOf2;
    table.resize(numEntries);
}

void TranspositionTable::Clear() {
    // A simple way to clear is to invalidate all entries
    // This is faster than reallocating memory
    for (auto& entry : table) {
        entry.isValid = false;
    }
}

TTEntry* TranspositionTable::Probe(uint64_t key) {
    if (numEntries == 0) return nullptr;

    size_t index = key & (numEntries - 1);
    TTEntry* entry = &table[index];

    // Check if the entry is valid and the Zobrist key matches (to handle collisions)
    if (entry->isValid && entry->key == key) {
        return entry;
    }

    return nullptr;
}

void TranspositionTable::Store(uint64_t key, int depth, int score, NodeType flag, Move bestMove) {
    if (numEntries == 0) return;

    size_t index = key & (numEntries - 1);
    TTEntry* entry = &table[index];

    // Using a simple "always replace" strategy.
    // A more advanced strategy might be to replace only if the new entry has a greater depth.
    entry->key = key;
    entry->depth = depth;
    entry->score = score;
    entry->flag = flag;
    entry->bestMove = bestMove;
    entry->isValid = true;
}
