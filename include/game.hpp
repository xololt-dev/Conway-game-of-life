#pragma once

#include "matrix.hpp"
#include "data.hpp"
#include "sync.hpp"

#include <ncurses.h>
#include <atomic>
#include <memory>
#include <vector>
#include <thread>

class Game {
public:
    void start();
    void pause();
    void resume();
    void restart();

    void workerFunction(const int a_id);
    void inputFunction(std::atomic_char& a_char);

    Game(short const a_rows) {
        data.currentGen = std::make_shared<Matrix<short>>(a_rows, a_rows);
        data.nextGen = std::make_shared<Matrix<short>>(a_rows, a_rows);

        hintPreAllocate();
    }

    Game(short const a_rows, short const a_columns) {
        data.currentGen = std::make_shared<Matrix<short>>(a_rows, a_columns);
        data.nextGen = std::make_shared<Matrix<short>>(a_rows, a_columns);

        hintPreAllocate();
    }

    void hintPreAllocate() {
        // Try to prealocate memory based on hardware hint
        unsigned int numThreads = std::thread::hardware_concurrency();

        if (numThreads)
            workers.reserve(numThreads);        
    }

private:
    Data data;
    Sync sync;

    std::vector<std::thread> workers;
};