#include "Engine.h"
#include <cstdlib>
#include <ctime>

int main() {
    srand(time(NULL));
    Engine uciEngine;
    uciEngine.run();
    return 0;
}
