#ifndef MAGIC_BITBOARDS_H
#define MAGIC_BITBOARDS_H

#include "DataTypes.h"
#include "Bitboard.h"

namespace MagicBitboards {

    // Initializes the magic bitboard tables.
    // Must be called once at the start of the program.
        void Init();

    // Gets rook attacks for a given square and occupancy.
    Bitboard get_rook_attacks(Square s, Bitboard occupancy);

    // Gets bishop attacks for a given square and occupancy.
    Bitboard get_bishop_attacks(Square s, Bitboard occupancy);

    // Gets queen attacks for a given square and occupancy.
    Bitboard get_queen_attacks(Square s, Bitboard occupancy);

} // namespace MagicBitboards

#endif // MAGIC_BITBOARDS_H
