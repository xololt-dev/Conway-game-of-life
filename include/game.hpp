#pragma once

#include "matrix.hpp"
#include "task.hpp"

#include <ncurses.h>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <tuple>
#include <vector>
#include <thread>

class Game {
public:
    void start();
    void pause();
    void resume();
    void restart();

    void workerFunction(const int a_id);

    void print() {
        std::tuple<short, short> dims = currentGen->dimensions();

        std::cout << "Conway's game of life\nTick: " << tick << 
        "\nDimensions: " << std::get<0>(dims) << " " << std::get<1>(dims) << std::endl;
    }

    Game(short const a_rows) {
        currentGen = std::make_unique<Matrix<short>>(a_rows, a_rows);
        nextGen = std::make_unique<Matrix<short>>(a_rows, a_rows);

        hintPreAllocate();
    }

    Game(short const a_rows, short const a_columns) {
        currentGen = std::make_unique<Matrix<short>>(a_rows, a_columns);
        nextGen = std::make_unique<Matrix<short>>(a_rows, a_columns);

        hintPreAllocate();
    }

    void hintPreAllocate() {
        // Try to prealocate memory based on hardware hint
        unsigned int numThreads = std::thread::hardware_concurrency();

        if (numThreads)
            workers.reserve(numThreads);
    }

private:
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    unsigned long long tick = 0;

    // Tasks for threads
    std::mutex tasks_mutex;
    std::condition_variable cv;
    std::queue<Task> tasks;
    bool queueInUse = false;
    // Other threads
    std::atomic_bool stopThreads = ATOMIC_VAR_INIT(false);
    std::atomic_bool pauseThreads = ATOMIC_VAR_INIT(false);
    std::vector<std::thread> workers;
};