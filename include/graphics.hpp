#pragma once

#include "data.hpp"

#include <memory>
#include <ncurses.h>
#include <tuple>

class Graphics {
public:
    void fullGameRender();
    void renderStats();
    void renderLesserStats();
    void renderBoard();
    void renderLoadBoard();

    void clearAll() { clear(); wrefresh(board);
        wrefresh(stats); }

    Graphics(const std::tuple<short, short> a_coords) : 
        coords(a_coords) {
        // Ncurses init
        initscr();
        noecho();
        setlocale(LC_ALL, "");
    }

    ~Graphics() {
        delwin(board);
        delwin(stats);
        endwin();
    }

    void setData(std::shared_ptr<Data> a_data) { data = a_data; };
    void setWorkers(int& a_workers) { workers = &a_workers; };
    void setTasks(int& a_tasks) { tasks = &a_tasks; };
    void setXPart(short& a_part) { x_part = &a_part; };
    void setYPart(short& a_part) { y_part = &a_part; };
    void setTimer(double& a_timer) { timer = &a_timer; };
    void setCoords(std::tuple<short, short> a_coords) { coords = a_coords; };

private:
    WINDOW* board = nullptr, 
          * stats = nullptr;

    std::tuple<short, short> coords;
    std::shared_ptr<Data> data;
    int* workers, * tasks;
    short * x_part, * y_part;
    double* timer;

    bool updateLesser = true;
};