#include "../include/Init.h"
#include "../include/Zobrist.h"
#include "../include/MagicBitboards.h"

Initializer::Initializer() {
    Zobrist::Init();
    MagicBitboards::Init();
}

// This static instance will call the constructor before main() starts.
static Initializer initializer;
