#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "DataTypes.h"
#include <cstdint>
#include <vector>

// The type of node/score stored in a TT entry
enum class NodeType {
    EXACT,       // The score is the exact evaluation for the node
    LOWER_BOUND, // Fail-high, the score is at least this value
    UPPER_BOUND  // Fail-low, the score is at most this value
};

struct TTEntry {
    uint64_t key = 0;      // Zobrist key to verify position
    Move bestMove{};       // Best move found at this position
    int depth = -1;        // The depth of the search that stored this entry
    int score = 0;         // The evaluation score
    NodeType flag = NodeType::EXACT;
    bool isValid = false;  // Simple flag to check if the entry is populated
};

class TranspositionTable {
public:
    // Constructor to initialize the table with a given size in MB
    explicit TranspositionTable(size_t sizeInMB);

    // Clears the transposition table
    void Clear();

    // Probes the table for an entry with the given Zobrist key.
    // Returns a pointer to the entry if found and valid, otherwise nullptr.
    TTEntry* Probe(uint64_t key);

    // Stores a new entry in the table.
    // Uses a replacement strategy if the target bucket is already occupied.
    void Store(uint64_t key, int depth, int score, NodeType flag, Move bestMove);

private:
    std::vector<TTEntry> table;
    size_t numEntries;
};

#endif // TRANSPOSITION_TABLE_H
