#pragma once

#include "data.hpp"

#include <memory>
#include <ncurses.h>
#include <locale>
#include <tuple>

class Graphics {
public:
    void renderStats();
    void renderBoard();

    Graphics(const std::tuple<short, short> a_coords) : 
        coords(a_coords) {
        // Ncurses init
        initscr();
        noecho();
        setlocale(LC_ALL, "");

        board = newwin(std::get<0>(a_coords) + 1, std::get<1>(a_coords) + 1, 0, 0);
        stats = newwin(std::get<1>(a_coords), 25, 0, std::get<0>(a_coords) + 1);

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
    }

    ~Graphics() {
        endwin();
    }

    void setData(std::shared_ptr<Data> a_data) { data = a_data; };
    void setWorkers(int& a_workers) { workers = &a_workers; };
    void setTasks(int& a_tasks) { tasks = &a_tasks; };
    void setXPart(short& a_part) { x_part = &a_part; };
    void setYPart(short& a_part) { y_part = &a_part; };
    //void setPaused(std::atomic_bool& a_paused) { *paused = a_paused; };
    void setTimer(double& a_timer) { timer = &a_timer; };

private:
    WINDOW* board, 
          * stats;

    const std::tuple<short, short> coords;
    std::shared_ptr<Data> data;
    int* workers, * tasks;
    short * x_part, * y_part;
    //std::atomic_bool* paused;
    double* timer;
};