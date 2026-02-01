/*
 * Native test for ConnectFour edge detection bug
 *
 * Provides minimal mocks for Arduino dependencies to test ConnectFour logic
 */

#include <cstdint>
#include <memory>
#include <iostream>

// Include Game base class and ConnectFour
#include "../include/games/ConnectFour.hpp"

void test_diagonal_win_at_edge() {
    std::cout << "Test: Diagonal four-in-a-row with edge scan bug\n";
    std::cout << "  Sequence: 3,1,3,4,4,3,4,4,5,2,5,2 (columns)\n";

    auto game = createConnectFour();

    // This sequence creates a diagonal win for player 1 (yellow, plays on even moves)
    // The diagonal goes up-right across columns 1,2,3,4 at increasing row heights
    // When the final piece is placed at column 2, the diagonal scan reaches the edge
    //
    // Bug occurs in scan() function at line 60:
    // - When scanning diagonally, pieces at cols 1,2,3,4 form a diagonal
    // - Placing final piece at col 2 triggers bidirectional scan
    // - One scan direction finds continuous pieces until hitting the board edge
    // - At edge, cell gets masked to 0 by interior mask (line 47)
    // - Loop exits and returns 0 instead of count (line 60)
    // - Total count becomes insufficient, win not detected
    //
    // Expected: Win should be detected (count >= 3)
    // Actual: Win NOT detected due to lost count at edge

    game->onKey(6, 3, false); // col 3, player 0
    game->onKey(6, 1, false); // col 1, player 1 - diagonal piece (bottom)
    game->onKey(6, 3, false); // col 3, player 0
    game->onKey(6, 4, false); // col 4, player 1
    game->onKey(6, 4, false); // col 4, player 0
    game->onKey(6, 3, false); // col 3, player 1 - diagonal piece
    game->onKey(6, 4, false); // col 4, player 0
    game->onKey(6, 4, false); // col 4, player 1 - diagonal piece
    game->onKey(6, 5, false); // col 5, player 0
    game->onKey(6, 2, false); // col 2, player 1 - one below diagonal
    game->onKey(6, 5, false); // col 5, player 0
    game->onKey(6, 2, false); // col 2, player 1 - completes diagonal 1-2-3-4, BUG!

    bool moveAccepted = game->onKey(6, 6, false); // try to play after win

    if (!moveAccepted) {
        std::cout << "  PASS: Win detected (no more moves accepted)\n";
    } else {
        std::cout << "  FAIL: Win NOT detected\n";
    }
    std::cout << "\n";
}

void test_horizontal_win_not_at_edge() {
    std::cout << "Test: Horizontal four-in-a-row NOT at edge (control)\n";

    auto game = createConnectFour();

    // Build horizontal in middle: columns 2,3,4,5
    game->onKey(6, 2, false); // col 2, player 0
    game->onKey(6, 0, false); // col 0, player 1
    game->onKey(6, 3, false); // col 2, player 0
    game->onKey(6, 0, false); // col 0, player 1
    game->onKey(6, 4, false); // col 4, player 0
    game->onKey(6, 0, false); // col 0, player 1
    game->onKey(6, 5, false); // col 5, player 0 - wins!

    bool moveAccepted = game->onKey(6, 1, false);

    if (!moveAccepted) {
        std::cout << "  PASS: Win detected\n";
    } else {
        std::cout << "  FAIL: Win NOT detected\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=== ConnectFour Edge Bug Test ===\n\n";
    std::cout << "Bug: scan() returned 0 instead of count when reaching board edge\n";
    std::cout << "Result: Four-in-a-row extending to edge was not detected as win\n\n";

    test_diagonal_win_at_edge();
    test_horizontal_win_not_at_edge();

    std::cout << "=== Test Complete ===\n";
    return 0;
}
