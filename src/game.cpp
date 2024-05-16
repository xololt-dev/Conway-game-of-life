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
    (*data.currentGen.get())(3, 3) = 1;
    (*data.currentGen.get())(3, 4) = 1;
    (*data.currentGen.get())(3, 5) = 1;
    // (*data.currentGen.get())(4, 4) = 1;

    // Setup
    sync.pauseThreads = true;

    // Detaching thread(s)
    sync.workDone.push_back(0);
    workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));
    workers.back().detach();

    // Task creation
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    short x_part = std::get<0>(data.currentGen->dimensions());
    short y_part = std::get<1>(data.currentGen->dimensions());

    std::tuple<short, short> coords = data.currentGen->dimensions();
    for (short x = 0; x < std::get<0>(coords); x += x_part) {
        for (short y = 0; y < std::get<1>(coords); y += y_part) {
            data.tasks.push(Task(data.currentGen, 
                data.nextGen, std::array<short, 4>{x, y, 
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
    mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", data.tick);
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

    int currentTasksAmount = 0;

    // Loop
    while (true) {
        sync.pauseThreads = false;

        // Quit
        if (input == 'q') {
            break;
        }
        // Add thread
        else if (input == '+' or input == '=') {
            sync.pauseThreads = true;
            
            sync.workDone.push_back(0);
            workers.push_back(std::thread(&Game::workerFunction, this, workers.size()));
            workers.back().detach();
            
            tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

            x_part = std::get<0>(data.currentGen->dimensions());
            y_part = std::get<1>(data.currentGen->dimensions());

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
            sync.pauseThreads = true;
        }
        else if (input == 'r') {
            sync.pauseThreads = false;
            input = ' ';
        }

        std::unique_lock<std::mutex> uniqueLock(sync.tasksMutex);
        sync.cvTasks.wait(uniqueLock, [this]{ return !sync.queueInUse; });

        if (!sync.queueInUse) {
            sync.queueInUse = true;

            bool canContinue = true;
            for (short& done : sync.workDone) {
                if (done == 0) {
                    canContinue = false;
                    break;
                }
            }

            if (data.tasks.empty() && canContinue) {
                sync.pauseThreads = true;

                mvwprintw(stats, 9, 4, "%s %f", "Timer: ",  
                double(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()));

                start = std::chrono::system_clock::now();

                // Swap 
                data.currentGen->load(data.nextGen->board());

                // Assign new tasks
                std::tuple<short, short> coords = data.currentGen->dimensions();
                unsigned short id = 0;

                for (short x = 0; x < std::get<0>(coords); x += x_part) {
                    for (short y = 0; y < std::get<1>(coords); y += y_part) {
                        data.tasks.push(Task(data.currentGen, 
                            data.nextGen, std::array<short, 4>{x, y, 
                            static_cast<short>(x + x_part - 1), static_cast<short>(y + y_part - 1)},
                            id++));
                    }
                }

                // Display
                setlocale(LC_ALL, "");
                for (int x = 0; x < std::get<0>(coords); x++) {
                    for (int y = 0; y < std::get<1>(coords); y++) {
                        mvwprintw(board, x + 1, y + 1, "%s", 
                            (*data.currentGen.get())(x, y) ? "@" : " ");
                    }
                }

                data.tick++;
            }
            else currentTasksAmount = data.tasks.size();

            sync.queueInUse = false;
        }

        // Refresh display data
        mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", data.tick);
        mvwprintw(stats, 3, 4, "%s %zu", "Workers: ", workers.size());
        mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", tasksAmount);
        mvwprintw(stats, 5, 4, "%s %d", "Current tasks: ", currentTasksAmount);
        mvwprintw(stats, 6, 4, "%s %d", "x_part: ", x_part);
        mvwprintw(stats, 7, 4, "%s %d", "y_part: ", y_part);
        mvwprintw(stats, 8, 4, "%s %d", "Pause: ", int(sync.pauseThreads));

        uniqueLock.unlock();
        sync.cvTasks.notify_all();

        wrefresh(board);
        wrefresh(stats);
    }

    sync.stopThreads = true;
    sync.pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    endwin();
}

void Game::workerFunction(const int a_id) {
    while (!sync.stopThreads) {
        while (!sync.pauseThreads) {
            std::unique_lock<std::mutex> uniqueLock(sync.tasksMutex);
            // W8 for our turn
            sync.cvTasks.wait(uniqueLock, [this]{ return !sync.queueInUse; });

            Task task;
            bool taskExists = false;

            if (!sync.queueInUse) {
                sync.queueInUse = true;
                
                if (!data.tasks.empty()) {
                    sync.workDone[a_id] = 0;

                    task = data.tasks.front();
                    taskExists = true;
                    
                    data.tasks.pop();
                }
                else sync.workDone[a_id] = 1;

                sync.queueInUse = false;
            }
            
            uniqueLock.unlock();
            sync.cvTasks.notify_one();

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
                        short aliveCount = (*data.currentGen.get())(left, top);
                        aliveCount += (*data.currentGen.get())(x, top);
                        aliveCount += (*data.currentGen.get())(right, top);

                        aliveCount += (*data.currentGen.get())(left, y);
                        aliveCount += (*data.currentGen.get())(right, y);

                        aliveCount += (*data.currentGen.get())(left, bottom);
                        aliveCount += (*data.currentGen.get())(x, bottom);
                        aliveCount += (*data.currentGen.get())(right, bottom);

                        // If alive
                        if ((*data.currentGen.get())(x, y) == 1) {
                            if (aliveCount < 2)
                                (*data.nextGen.get())(x,y) = 0;
                            else if (aliveCount > 3)
                                (*data.nextGen.get())(x,y) = 0;
                            else
                                (*data.nextGen.get())(x,y) = 1;
                        }
                        // If dead
                        else {
                            if (aliveCount == 3)
                                (*data.nextGen.get())(x,y) = 1;
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
    while (!sync.stopThreads)
        a_char = getch();
}