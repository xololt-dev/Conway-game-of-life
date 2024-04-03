#include "game.hpp"
#include "task.hpp"

#include <chrono>
#include <ncurses.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>

void Game::start() {
    // Setup
    pauseThreads = true;
    workers.push_back(std::thread(&Game::workerFunction, this, 1));
    workers.push_back(std::thread(&Game::workerFunction, this, 2)); 

    //TODO: how to divide the matrix into tasks?
    tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 0, 100, 50)));
    tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 50, 100, 100)));

    initscr();

    std::cout << "Before detach\n";

    for (std::thread& w : workers) {
        w.detach();
    }
    pauseThreads = false;

    printw("After detach\n");

    std::chrono::time_point<std::chrono::system_clock> a = std::chrono::system_clock::now(); 

    // Loop
    while (std::chrono::system_clock::now() - a < std::chrono::seconds(5)) {
        if (tasks.empty()) {
            // Swap 
            currentGen = nextGen;

            // New tasks
            // log2
            tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 0, 100, 50)));
            tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 50, 100, 100)));
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    stopThreads = true;
    pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Game::workerFunction(int a_id) {
    while (!stopThreads) {
        while (!pauseThreads) {
            // W8 for our turn
            std::unique_lock<std::mutex> uniqueLock(tasks_mutex);
            cv.wait(uniqueLock, [this]{return !queueInUse;});

            queueInUse = true;
            Task* task = nullptr;

            if (!tasks.empty()) {
                task = &tasks.front();

                std::cout << "Worker #" << a_id << "\n" 
                << std::get<0>(task->coordinates) << " "
                << std::get<1>(task->coordinates) << " "
                << std::get<2>(task->coordinates) << " "
                << std::get<3>(task->coordinates) << std::endl;
                
                tasks.pop();
            }
            else {
                std::cout << "No task!: " << a_id << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            uniqueLock.unlock();
            queueInUse = false;
            cv.notify_all();

            // Do work here - seperate function?
            for (unsigned int x = std::get<0>(task->coordinates); 
                x < std::get<2>(task->coordinates); x++) {
                for (unsigned int y = std::get<1>(task->coordinates);
                    y < std::get<3>(task->coordinates); y++) {
                
                }
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Thread ends!\n";
}