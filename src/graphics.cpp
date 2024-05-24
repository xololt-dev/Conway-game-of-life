#include "graphics.hpp"

void Graphics::renderStats() {
    // Refresh display data
    mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", data->tick);
    mvwprintw(stats, 3, 13, "%s", "   ");
    mvwprintw(stats, 3, 4, "%s %d", "Workers: ", *workers);
    mvwprintw(stats, 4, 11, "%s", "   ");
    mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", *tasks);
    // mvwprintw(stats, 5, 4, "%s %d", "Current tasks: ", currentTasksAmount);
    mvwprintw(stats, 6, 12, "%s", "   ");
    mvwprintw(stats, 6, 4, "%s %d", "x_part: ", *x_part);
    mvwprintw(stats, 7, 12, "%s", "   ");
    mvwprintw(stats, 7, 4, "%s %d", "y_part: ", *y_part);
    //mvwprintw(stats, 8, 4, "%s %d", "Pause: ", *paused);
    mvwprintw(stats, 9, 4, "%s %f", "Timer: ", *timer);

    wrefresh(stats);
}

void Graphics::renderBoard() {
    // Display
    for (int x = 0; x < std::get<0>(coords); x++) {
        for (int y = 0; y < std::get<1>(coords); y++) {
            mvwprintw(board, x + 1, y + 1, "%s", 
                (*data->currentGen.get())(x, y) ? "@" : " ");
        }
    }

    wrefresh(board);
}