#include "game.hpp"

int main(int argc, const char * argv[]) {
    Game game(72, 344);
    //Game game(32);
    //Game game(1080*2, 1920*2);
    game.start();
    return 0;
}