#include "game.hpp"
#include "task.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <memory>
#include <ncurses.h>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

void Game::start() {
    (*currentGen.get())(3, 3) = 1;
    (*currentGen.get())(3, 4) = 1;
    (*currentGen.get())(3, 5) = 1;

    // Setup
    pauseThreads = true;

    // Detaching thread(s) 
    workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));
    workers.back().detach();

    // Task creation
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    short x_part = std::get<0>(currentGen->dimensions());
    short y_part = std::get<1>(currentGen->dimensions());

    std::tuple<short, short> coords = currentGen->dimensions();
    for (short x = 0; x < std::get<0>(coords); x += x_part) {
        for (short y = 0; y < std::get<1>(coords); y += y_part) {
            tasks.push(Task(currentGen, 
                nextGen, std::array<short, 4>{x, y, 
                static_cast<short>(x + x_part - 1), static_cast<short>(y + y_part - 1)},
                0));
        }
    }

    // Ncurses init
    initscr();
    noecho();
    
    // Window creation
    WINDOW* board = newwin(std::get<0>(coords) + 1, std::get<1>(coords) + 1, 0, 0);
    WINDOW* stats = newwin(std::get<1>(coords), 25, 0, std::get<0>(coords) + 1);
    
    refresh();

    box(board, 0, 0);
    box(stats, 0, 0);
    refresh();

    // move and print in window
    mvwprintw(board, 0, 1, "Conway's game of life");
    
    mvwprintw(stats, 0, 1, "Statistics");
    mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", tick);
    mvwprintw(stats, 3, 4, "%s %zu", "Workers: ", workers.size());
    mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", tasksAmount);

    // refreshing the window
    wrefresh(board);
    wrefresh(stats);

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    // Input thread
    std::atomic_char input(' ');
    std::thread inputThread = std::thread(&Game::inputFunction, this, std::ref(input));
    inputThread.detach();

    // Loop
    while (true) {
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
            
            tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

            x_part = std::get<0>(currentGen->dimensions());
            y_part = std::get<1>(currentGen->dimensions());

            if (tasksAmount > 1) {
                bool x_bool = true;
                for (int i = tasksAmount; i > 1; i /= 2) {
                    if (x_bool)
                        x_part /= 2;
                    else
                        y_part /= 2;

                    x_bool = !x_bool;
                }
            }

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

            x_part = std::get<0>(currentGen->dimensions());
            y_part = std::get<1>(currentGen->dimensions());

            if (tasksAmount > 1) {
                for (int i = tasksAmount; i > 1; i /= 2) {
                    if (i % 2)
                        y_part /= 2;
                    else
                        x_part /= 2;
                }
            }

            input = ' ';
        }
        */
        else if (input == 'p') {
            pauseThreads = true;
        }
        else if (input == 'r') {
            pauseThreads = false;
            input = ' ';
        }

        std::unique_lock<std::mutex> uniqueLock(tasks_mutex);
        cv_tasks.wait(uniqueLock, [this]{ return !queueInUse; });
        queueInUse = true;

        if (tasks.empty()) {
            pauseThreads = true;

            mvwprintw(stats, 8, 4, "%s %f", "Timer: ",  
            double(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()));

            start = std::chrono::system_clock::now();

            // Swap 
            currentGen->load(nextGen->board());

            // Assign new tasks
            std::tuple<short, short> coords = currentGen->dimensions();
            unsigned short id = 0;

            for (short x = 0; x < std::get<0>(coords); x += x_part) {
                for (short y = 0; y < std::get<1>(coords); y += y_part) {
                    tasks.push(Task(currentGen, 
                        nextGen, std::array<short, 4>{x, y, 
                        static_cast<short>(x + x_part - 1), static_cast<short>(y + y_part - 1)},
                        id++));
                }
            }

            // Display
            setlocale(LC_ALL, "");
            for (int x = 0; x < std::get<0>(coords); x++) {
                for (int y = 0; y < std::get<1>(coords); y++) {
                    mvwprintw(board, x + 1, y + 1, "%s", 
                        (*currentGen.get())(x, y) ? "@" : " ");
                }
            }

            tick++;
        }

        // Refresh display data
        mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", tick);
        mvwprintw(stats, 3, 4, "%s %zu", "Workers: ", workers.size());
        mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", tasksAmount);
        mvwprintw(stats, 5, 4, "%s %d", "x_part: ", x_part);
        mvwprintw(stats, 6, 4, "%s %d", "y_part: ", y_part);
        mvwprintw(stats, 7, 4, "%s %d", "Pause: ", int(pauseThreads));

        wrefresh(board);
        wrefresh(stats);

        uniqueLock.unlock();
        queueInUse = false;
        cv_tasks.notify_all();

        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
            cv_tasks.wait(uniqueLock, [this]{ return !queueInUse; });

            queueInUse = true;
            Task task;
            bool taskExists = false;

            if (!tasks.empty()) {
                task = tasks.front();
                taskExists = true;
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
            cv_tasks.notify_all();

            // Do work here - seperate function?
            if (taskExists) {
                for (short x = task.coordinates[0]; 
                    x <= task.coordinates[2]; x++) {
                    int left = x - 1;
                    int right = x + 1;

                    if (x == 0)
                        left = std::get<0>(task.currentGen->dimensions()) - 1;
                    else if (x == std::get<0>(task.currentGen->dimensions()) - 1)
                        right = 0;

                    for (short y = task.coordinates[1];
                        y <= task.coordinates[3]; y++) {
                        // Edges
                        int bottom = y + 1;
                        int top = y - 1;

                        if (y == 0)
                            top = std::get<1>(task.currentGen->dimensions()) - 1;
                        else if (y == std::get<1>(task.currentGen->dimensions()) - 1)
                            bottom = 0;

                        // Count alive neighbours
                        short aliveCount = (*currentGen.get())(left, top);
                        aliveCount += (*currentGen.get())(x, top);
                        aliveCount += (*currentGen.get())(right, top);

                        aliveCount += (*currentGen.get())(left, y);
                        aliveCount += (*currentGen.get())(right, y);

                        aliveCount += (*currentGen.get())(left, bottom);
                        aliveCount += (*currentGen.get())(x, bottom);
                        aliveCount += (*currentGen.get())(right, bottom);

                        // If alive
                        if ((*currentGen.get())(x, y) == 1) {
                            if (aliveCount < 2)
                                (*nextGen.get())(x,y) = 0;
                            else if (aliveCount > 3)
                                (*nextGen.get())(x,y) = 0;
                            else
                                (*nextGen.get())(x,y) = 1;
                        }
                        // If dead
                        else {
                            if (aliveCount == 3)
                                (*nextGen.get())(x,y) = 1;
                        }
                    }
                }
            }
        }

        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

void Game::inputFunction(std::atomic_char& a_char) {
    while (!stopThreads)
        a_char = getch();
}