#include "test_helpers.h"
#include <iostream>

// Forward declarations for test runners
void run_move_tests();
void run_board_tests();
void run_fen_tests();
void run_opening_tests();

int main() {
    std::cout << "=====================" << std::endl;
    std::cout << "  RUNNING ALL TESTS  " << std::endl;
    std::cout << "=====================" << std::endl;

    try {
        run_fen_tests();
        run_board_tests();
        run_move_tests();
        run_opening_tests();
    } catch (const AssertException& e) {
        std::cerr << "\nA test case failed. Aborting." << std::endl;
        return 1;
    }



    std::cout << "\n=====================" << std::endl;
    std::cout << " ALL TESTS PASSED! " << std::endl;
    std::cout << "=====================" << std::endl;

    return 0;
}