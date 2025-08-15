#include "../include/MagicBitboards.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <random>

namespace {

// Pre-calculated magic numbers. In a real application, these would be
// either found at runtime or pre-calculated and stored. For simplicity,
// we use pre-calculated values found by the community.
const uint64_t rook_magics[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100008100040002ULL,
    0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004001000ULL,
    0x800091400100100ULL, 0x4010002001080200ULL, 0x80080200010100ULL, 0x804000400080400ULL,
    0x4008010010004ULL, 0x2000100200801ULL, 0x100400080208000ULL, 0x4000020800400ULL,
    0x40080800800800ULL, 0x800400200040100ULL, 0x8000401000200040ULL, 0x200100800802000ULL,
    0x100040002000400ULL, 0x200040010020080ULL, 0x4000100080800ULL, 0x8000080200100080ULL,
    0x8000000004080100ULL, 0x800004010002000ULL, 0x1000000080020ULL, 0x2000000401000ULL,
    0x400000000004008ULL, 0x8000001004000200ULL, 0x800000000100040ULL, 0x800000000800800ULL,
    0x100000000020008ULL, 0x800000000040020ULL, 0x800000000200040ULL, 0x800000000080010ULL,
    0x100000800100020ULL, 0x400000000080080ULL, 0x800000000010010ULL, 0x800000000400020ULL,
    0x800000000800040ULL, 0x800000000100020ULL, 0x400000000080010ULL, 0x400000000200008ULL,
    0x800000000400010ULL, 0x800000000080020ULL, 0x800000000010040ULL, 0x800000000200080ULL,
    0x800000000400080ULL, 0x800000000080010ULL, 0x100000000040008ULL, 0x200000000100004ULL,
    0x400000000080002ULL, 0x800000000100004ULL, 0x800000000400008ULL, 0x400000000080001ULL,
    0x800000000100002ULL, 0x800000000400004ULL, 0x200000000800001ULL, 0x100000000400002ULL
};

const uint64_t bishop_magics[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x101010101010101ULL, 0x202020202020202ULL,
    0x404040404040404ULL, 0x808080808080808ULL, 0x102040810204080ULL, 0x2040810204082040ULL,
    0x4081020408204080ULL, 0x8102040820408081ULL, 0x1020408204080810ULL, 0x2040820408081020ULL,
    0x4082040808102040ULL, 0x8204080810204080ULL, 0x4080810204080ULL, 0x8081020408080ULL,
    0x102040808080ULL, 0x204080808080ULL, 0x408080808080ULL, 0x808080808080ULL,
    0x10204080808ULL, 0x20408080808ULL, 0x40808080808ULL, 0x80808080808ULL,
    0x1020408080ULL, 0x2040808080ULL, 0x4080808080ULL, 0x8080808080ULL,
    0x102040808ULL, 0x204080808ULL, 0x408080808ULL, 0x808080808ULL,
    0x10204080ULL, 0x20408080ULL, 0x40808080ULL, 0x80808080ULL,
    0x1020408ULL, 0x2040808ULL, 0x4080808ULL, 0x8080808ULL,
    0x10204ULL, 0x20408ULL, 0x40808ULL, 0x80808ULL,
    0x102ULL, 0x204ULL, 0x408ULL, 0x808ULL,
    0x1ULL, 0x2ULL, 0x4ULL, 0x8ULL,
    0x10ULL, 0x20ULL, 0x40ULL, 0x80ULL,
    0x100ULL, 0x200ULL, 0x400ULL, 0x800ULL,
    0x1000ULL, 0x2000ULL, 0x4000ULL, 0x8000ULL
};

// Attack tables
Bitboard rook_attacks[64][4096];
Bitboard bishop_attacks[64][512];

// Occupancy masks
Bitboard rook_masks[64];
Bitboard bishop_masks[64];

// Relevant bit counts for masks
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// Generates the occupancy mask for a rook on a given square.
Bitboard mask_rook_attacks(int sq) {
    Bitboard result = 0ULL;
    int r = sq / 8;
    int f = sq % 8;
    for (int i = r + 1; i <= 7; i++) result |= (1ULL << (i * 8 + f));
    for (int i = r - 1; i >= 0; i--) result |= (1ULL << (i * 8 + f));
    for (int i = f + 1; i <= 7; i++) result |= (1ULL << (r * 8 + i));
    for (int i = f - 1; i >= 0; i--) result |= (1ULL << (r * 8 + i));
    return result;
}

// Generates the occupancy mask for a bishop on a given square.
Bitboard mask_bishop_attacks(int sq) {
    Bitboard result = 0ULL;
    int r = sq / 8;
    int f = sq % 8;
    for (int i = r + 1, j = f + 1; i <= 7 && j <= 7; i++, j++) result |= (1ULL << (i * 8 + j));
    for (int i = r + 1, j = f - 1; i <= 7 && j >= 0; i++, j--) result |= (1ULL << (i * 8 + j));
    for (int i = r - 1, j = f + 1; i >= 0 && j <= 7; i--, j++) result |= (1ULL << (i * 8 + j));
    for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) result |= (1ULL << (i * 8 + j));
    return result;
}

//Bitboard mask_rook_attacks(int sq) {
//    Bitboard result = 0ULL;
//    int r = sq / 8;
//    int f = sq % 8;
//    for (int i = r + 1; i <= 6; i++) result |= (1ULL << (i * 8 + f));
//    for (int i = r - 1; i >= 1; i--) result |= (1ULL << (i * 8 + f));
//    for (int i = f + 1; i <= 6; i++) result |= (1ULL << (r * 8 + i));
//    for (int i = f - 1; i >= 1; i--) result |= (1ULL << (r * 8 + i));
//    return result;
//}
//
//Bitboard mask_bishop_attacks(int sq) {
//    Bitboard result = 0ULL;
//    int r = sq / 8;
//    int f = sq % 8;
//    for (int i = r + 1, j = f + 1; i <= 6 && j <= 6; i++, j++) result |= (1ULL << (i * 8 + j));
//    for (int i = r + 1, j = f - 1; i <= 6 && j >= 1;  i++, j--) result |= (1ULL << (i * 8 + j));
//    for (int i = r - 1, j = f + 1; i >= 1 && j <= 6;  i--, j++) result |= (1ULL << (i * 8 + j));
//    for (int i = r - 1, j = f - 1; i >= 1 && j >= 1;  i--, j--) result |= (1ULL << (i * 8 + j));
//    return result;
//}


// Generates rook attacks on the fly (the slow way, for initialization).
Bitboard rook_attacks_on_the_fly(int sq, Bitboard block) {
    Bitboard result = 0ULL;
    int r = sq / 8;
    int f = sq % 8;
    for (int i = r + 1; i <= 7; i++) {
        result |= (1ULL << (i * 8 + f));
        if (block & (1ULL << (i * 8 + f))) break;
    }
    for (int i = r - 1; i >= 0; i--) {
        result |= (1ULL << (i * 8 + f));
        if (block & (1ULL << (i * 8 + f))) break;
    }
    for (int i = f + 1; i <= 7; i++) {
        result |= (1ULL << (r * 8 + i));
        if (block & (1ULL << (r * 8 + i))) break;
    }
    for (int i = f - 1; i >= 0; i--) {
        result |= (1ULL << (r * 8 + i));
        if (block & (1ULL << (r * 8 + i))) break;
    }
    return result;
}

// Generates bishop attacks on the fly.
Bitboard bishop_attacks_on_the_fly(int sq, Bitboard block) {
    Bitboard result = 0ULL;
    int r = sq / 8;
    int f = sq % 8;
    for (int i = r + 1, j = f + 1; i <= 7 && j <= 7; i++, j++) {
        result |= (1ULL << (i * 8 + j));
        if (block & (1ULL << (i * 8 + j))) break;
    }
    for (int i = r + 1, j = f - 1; i <= 7 && j >= 0; i++, j--) {
        result |= (1ULL << (i * 8 + j));
        if (block & (1ULL << (i * 8 + j))) break;
    }
    for (int i = r - 1, j = f + 1; i >= 0 && j <= 7; i--, j++) {
        result |= (1ULL << (i * 8 + j));
        if (block & (1ULL << (i * 8 + j))) break;
    }
    for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) {
        result |= (1ULL << (i * 8 + j));
        if (block & (1ULL << (i * 8 + j))) break;
    }
    return result;
}

// Generates all blocker combinations for a given mask.
Bitboard set_occupancy(int index, int bits_in_mask, Bitboard attack_mask) {
    Bitboard occupancy = 0ULL;
    Bitboard temp_attack_mask = attack_mask;
    for (int i = 0; i < bits_in_mask; i++) {
        int sq = __builtin_ctzll(temp_attack_mask);
        if (index & (1 << i)) {
            occupancy |= (1ULL << sq);
        }
        temp_attack_mask &= temp_attack_mask - 1;
    }
    return occupancy;
}

} // anonymous namespace

namespace MagicBitboards {

void Init() {
    for (int sq = 0; sq < 64; sq++) {
        // Rook initialization
        rook_masks[sq] = mask_rook_attacks(sq);
        int rook_bit_count = __builtin_popcountll(rook_masks[sq]);
        int rook_occupancy_variations = 1 << rook_bit_count;
        for (int i = 0; i < rook_occupancy_variations; i++) {
            Bitboard occupancy = set_occupancy(i, rook_bit_count, rook_masks[sq]);
            int magic_index = (occupancy * rook_magics[sq]) >> (64 - rook_relevant_bits[sq]);
            rook_attacks[sq][magic_index] = rook_attacks_on_the_fly(sq, occupancy);
        }

        // Bishop initialization
        bishop_masks[sq] = mask_bishop_attacks(sq);
        int bishop_bit_count = __builtin_popcountll(bishop_masks[sq]);
        int bishop_occupancy_variations = 1 << bishop_bit_count;
        for (int i = 0; i < bishop_occupancy_variations; i++) {
            Bitboard occupancy = set_occupancy(i, bishop_bit_count, bishop_masks[sq]);
            int magic_index = (occupancy * bishop_magics[sq]) >> (64 - bishop_relevant_bits[sq]);
            bishop_attacks[sq][magic_index] = bishop_attacks_on_the_fly(sq, occupancy);
        }
    }
    
}

Bitboard get_rook_attacks(Square s, Bitboard occupancy) {
    int sq_idx = s.rank * 8 + s.file;
    occupancy &= rook_masks[sq_idx];
    int magic_index = (occupancy * rook_magics[sq_idx]) >> (64 - rook_relevant_bits[sq_idx]);
    return rook_attacks[sq_idx][magic_index];
}

Bitboard get_bishop_attacks(Square s, Bitboard occupancy) {
    int sq_idx = s.rank * 8 + s.file;
    occupancy &= bishop_masks[sq_idx];
    int magic_index = (occupancy * bishop_magics[sq_idx]) >> (64 - bishop_relevant_bits[sq_idx]);
    return bishop_attacks[sq_idx][magic_index];
}

Bitboard get_queen_attacks(Square s, Bitboard occupancy) {
    return get_rook_attacks(s, occupancy) | get_bishop_attacks(s, occupancy);
}

} // namespace MagicBitboards
