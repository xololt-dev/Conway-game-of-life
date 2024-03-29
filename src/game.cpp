#include "game.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>

void Game::start() {
    // Setup
    pauseThreads = true;
    workers.push_back(std::thread(&Game::workerFunction, this));

    std::cout << "Before detach\n"; 

    for (std::thread& w : workers) {
        w.detach();
    }
    pauseThreads = false;

    //TODO: how to divide the matrix into tasks?
    tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 0, 0, 0)));

    std::cout << "After detach\n"; 

    auto a = std::chrono::system_clock::now(); 

    // Loop
    while (std::chrono::system_clock::now() - a < std::chrono::seconds(10)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    stopThreads = true;
    pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
}

void Game::workerFunction() {
    while (!stopThreads) {
        while (!pauseThreads) {
            // W8 for our turn
            std::unique_lock<std::mutex> uniqueLock(tasks_mutex);
            cv.wait(uniqueLock, [this]{return !queueInUse;});

            queueInUse = true;
            Task* task = nullptr;

            if (!tasks.empty()) {
                task = &tasks.front();
                std::cout << std::get<0>(task->coordinates) << std::endl;
                tasks.pop();
            }
            // else 
                //std::cout << "No task!\n";

            uniqueLock.unlock();
            queueInUse = false;
            cv.notify_all();
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Thread ends!\n";
}