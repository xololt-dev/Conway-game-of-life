#include "game.hpp"
#include "task.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <ncurses.h>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>

void Game::start() {
    // Setup
    pauseThreads = true;
    workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));

    //TODO: how to divide the matrix into tasks?
    int tasksAmount = std::pow(2, std::ceil(log(workers.size())));

    tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 0, 50, 25)));
    tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 25, 50, 50)));

    // Ncurses init
    initscr();
    noecho();
    
    // Window creation
    WINDOW* board = newwin(50, 50, 0, 0);
    WINDOW* stats = newwin(50, 25, 0, 50);
    
    refresh();

    box(board, 0, 0);
    box(stats, 0, 0);
    refresh();

    // move and print in window
    mvwprintw(board, 0, 1, "Conway's game of life");
    
    mvwprintw(stats, 0, 1, "Statistics");
    mvwprintw(stats, 2, 4, "%s", ("Tick: " + std::to_string(tick)).c_str());
    mvwprintw(stats, 3, 4, "%s", ("Workers: " + std::to_string(workers.size())).c_str());
    mvwprintw(stats, 4, 4, "%s", ("Tasks: " + std::to_string(tasksAmount)).c_str());

    // refreshing the window
    wrefresh(board);
    wrefresh(stats);

    // Detaching thread(s)
    for (std::thread& w : workers) {
        w.detach();
    }

    std::chrono::time_point<std::chrono::system_clock> a = std::chrono::system_clock::now(); 

    // Input thread
    std::atomic_char input('p');
    std::thread inputThread = std::thread(&Game::inputFunction, this, std::ref(input));
    inputThread.detach();

    // Loop
    while (std::chrono::system_clock::now() - a < std::chrono::seconds(10)) {
        pauseThreads = false;

        // Quit
        if (input == 'q') {
            break;
        }
        // Add thread
        else if (input == '+' or input == '=') {
            pauseThreads = true;
            
            workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));

            workers.back().detach();

            tasksAmount = std::pow(2, std::ceil(log2(double(workers.size()))));

            input = ' ';
        }
        // Delete thread
        /*
        else if (input == '-') {
            // Always have at least one
            if (workers.size() <= 1)
                break;

            // Is it even safe bro? Does it even do it's job lmao?
            // workers.erase(workers.end());

            // When in doubt...
            int amountNeeded = workers.size() - 1;
            
            pauseThreads = true;
            stopThreads = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            workers.clear();
            for (int i = 0; i < amountNeeded; i++) {
                workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));
            }

            tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

            input = ' ';
        }
        */

        if (tasks.empty()) {
            pauseThreads = true;
            // Swap 
            currentGen = nextGen;

            // New tasks
            std::tuple<short, short> dims = currentGen->dimensions();

            // log2
            tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 0, 50, 25)));
            tasks.push(Task(currentGen, nextGen, std::make_tuple(0, 25, 50, 50)));

            tick++;
        }

        // Refresh display data
        mvwprintw(stats, 2, 4, "%s", ("Tick: " + std::to_string(tick)).c_str());
        mvwprintw(stats, 3, 4, "%s", ("Workers: " + std::to_string(workers.size())).c_str());
        mvwprintw(stats, 4, 4, "%s", ("Tasks: " + std::to_string(tasksAmount)).c_str());

        wrefresh(board);
        wrefresh(stats);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    stopThreads = true;
    pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    endwin();
}

void Game::workerFunction(const int a_id) {
    while (!stopThreads) {
        while (!pauseThreads) {
            // W8 for our turn
            std::unique_lock<std::mutex> uniqueLock(tasks_mutex);
            cv.wait(uniqueLock, [this]{return !queueInUse;});

            queueInUse = true;
            Task* task = nullptr;

            if (!tasks.empty()) {
                task = &tasks.front();
/*
                std::cout << "Worker #" << a_id << "\n" 
                << std::get<0>(task->coordinates) << " "
                << std::get<1>(task->coordinates) << " "
                << std::get<2>(task->coordinates) << " "
                << std::get<3>(task->coordinates) << std::endl;
*/                
                tasks.pop();
            }

            uniqueLock.unlock();
            queueInUse = false;
            cv.notify_all();

            // Do work here - seperate function?
            /*
            for (unsigned int x = std::get<0>(task->coordinates); 
                x < std::get<2>(task->coordinates); x++) {
                for (unsigned int y = std::get<1>(task->coordinates);
                    y < std::get<3>(task->coordinates); y++) {
                
                }
            }*/
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // std::cout << "Thread ends!\n";
}

void Game::inputFunction(std::atomic_char& a_char) {
    while (!stopThreads)
        a_char = getch();
}