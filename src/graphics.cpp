#include "graphics.hpp"
#include <ncurses.h>

void Graphics::fullGameRender() {
    if (board != nullptr)
        delwin(board);

    if (stats != nullptr)
        delwin(stats);

    board = newwin(std::get<0>(coords) + 3, std::get<1>(coords) + 2, 0, 0);
    stats = newwin(std::get<0>(coords) + 2, 25, 0, std::get<0>(coords) + 2);

    refresh();

    box(board, 0, 0);
    box(stats, 0, 0);
    refresh();

    // move and print in window
    mvwprintw(board, 0, 1, "Conway's game of life");
    mvwprintw(stats, 0, 1, "Statistics");

    // refreshing the window
    wrefresh(board);
    wrefresh(stats);

    renderStats();
    renderLesserStats();
    renderBoard();
}

void Graphics::renderStats() {
    // Refresh display data
    mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", data->tick);
    mvwprintw(stats, 9, 4, "%s %f", "Timer: ", *timer);

    wrefresh(stats);
}

void Graphics::renderLesserStats() {
    mvwprintw(stats, 3, 13, "%s", "   ");
    mvwprintw(stats, 3, 4, "%s %d", "Workers: ", *workers);
    mvwprintw(stats, 4, 11, "%s", "   ");
    mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", *tasks);
    // mvwprintw(stats, 5, 4, "%s %d", "Current tasks: ", currentTasksAmount);
    mvwprintw(stats, 6, 12, "%s", "   ");
    mvwprintw(stats, 6, 4, "%s %d", "x_part: ", *x_part);
    mvwprintw(stats, 7, 12, "%s", "   ");
    mvwprintw(stats, 7, 4, "%s %d", "y_part: ", *y_part);
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

void Graphics::renderLoadBoard() {
    if (board != nullptr)
        delwin(board);

    if (stats != nullptr)
        delwin(stats);
    
    board = newwin(9, 102, 0, 0);
    
    refresh();

    box(board, 0, 0);
    refresh();

    mvwprintw(board, 0, 1, "Conway's game of life");
    mvwprintw(board, 3, 3, "Podaj sciezke do pliku z wzorem:");
    mvwprintw(board, 5, 3, "                                ");
    if (data->pathToFile.length())
        mvwprintw(board, 5, 3, "%s", data->pathToFile.c_str());

    wrefresh(board);
}