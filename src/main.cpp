#include "game.hpp"

int main(int argc, const char * argv[]) {
    Game game(100);
    game.print();
    game.start();
    return 0;
}