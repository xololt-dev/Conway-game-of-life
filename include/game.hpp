#pragma once

#include "matrix.hpp"
#include "task.hpp"

#include <ncurses.h>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
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
        currentGen = std::make_shared<Matrix<short>>(a_rows, a_rows);
        nextGen = std::make_shared<Matrix<short>>(a_rows, a_rows);

        hintPreAllocate();
    }

    Game(short const a_rows, short const a_columns) {
        currentGen = std::make_shared<Matrix<short>>(a_rows, a_columns);
        nextGen = std::make_shared<Matrix<short>>(a_rows, a_columns);

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
    std::mutex tasksMutex;
    std::condition_variable cvTasks;
    std::queue<Task> tasks;
    std::atomic_bool queueInUse = ATOMIC_VAR_INIT(false);
    // Other threads
    std::atomic_bool stopThreads = ATOMIC_VAR_INIT(false);
    std::atomic_bool pauseThreads = ATOMIC_VAR_INIT(false);
    std::vector<std::thread> workers;
    std::vector<short> workDone;
};