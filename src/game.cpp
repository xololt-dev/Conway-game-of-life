#include "game.hpp"
#include "task.hpp"
#include "worker.hpp"

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
    (*data->currentGen.get())(3, 3) = 1;
    (*data->currentGen.get())(3, 4) = 1;
    (*data->currentGen.get())(3, 5) = 1;
    // (*data.currentGen.get())(4, 4) = 1;

    // Setup
    sync->pauseThreads = true;

    // Detaching thread(s)
    sync->workDone.push_back(0);
    workers.push_back(Worker(workers.size(), data, sync));    
    workers.back().detach();
    
    // Task creation
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    short x_part = std::get<0>(data->currentGen->dimensions());
    short y_part = std::get<1>(data->currentGen->dimensions());

    std::tuple<short, short> coords = data->currentGen->dimensions();
    for (short x = 0; x < std::get<0>(coords); x += x_part) {
        for (short y = 0; y < std::get<1>(coords); y += y_part) {
            data->tasks.push(Task(data->currentGen, 
                data->nextGen, std::array<short, 4>{x, y, 
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

    // refreshing the window
    wrefresh(board);
    wrefresh(stats);

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    // Input thread
    std::atomic_char input(' ');
    std::thread inputThread = std::thread(&Game::inputFunction, this, std::ref(input));
    inputThread.detach();

    int currentTasksAmount = 0;
    bool addThread = false;
    bool deleteThread = false;

    // Loop
    while (true) {
        sync->pauseThreads = false;

        // Quit
        if (input == 'q') {
            break;
        }
        // Add thread
        else if (input == '+' or input == '=') {
            if (deleteThread)
                deleteThread = false;
            else
                addThread = true;

            input = ' ';
        }
        // Delete thread
        else if (input == '-') {
            if (addThread)
                addThread = false;
            else
                deleteThread = true;
            
            input = ' ';
        }
        else if (input == 'p') {
            sync->pauseThreads = true;
        }
        else if (input == 'r') {
            sync->pauseThreads = false;
            input = ' ';
        }

        std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);
        sync->cvTasks.wait(uniqueLock, [this]{ return !sync->queueInUse; });

        if (!sync->queueInUse) {
            sync->queueInUse = true;

            bool canContinue = true;
            for (short& done : sync->workDone) {
                if (done == 0) {
                    canContinue = false;
                    break;
                }
            }

            if (data->tasks.empty() && canContinue) {
                sync->pauseThreads = true;

                if (deleteThread) {
                    deleteWorker();

                    std::tuple<int, short, short> tempTuple = getTasksSize();
                    tasksAmount = std::get<0>(tempTuple);
                    x_part = std::get<1>(tempTuple);
                    y_part = std::get<2>(tempTuple);
                }
                else if (addThread) {
                    addWorker();

                    std::tuple<int, short, short> tempTuple = getTasksSize();
                    tasksAmount = std::get<0>(tempTuple);
                    x_part = std::get<1>(tempTuple);
                    y_part = std::get<2>(tempTuple);
                }
                deleteThread = false;
                addThread = false;

                mvwprintw(stats, 9, 4, "%s %f", "Timer: ",  
                double(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()));

                start = std::chrono::system_clock::now();

                // Swap 
                data->currentGen->load(data->nextGen->board());

                // Assign new tasks
                std::tuple<short, short> coords = data->currentGen->dimensions();
                unsigned short id = 0;

                for (short x = 0; x < std::get<0>(coords); x += x_part) {
                    for (short y = 0; y < std::get<1>(coords); y += y_part) {
                        data->tasks.push(Task(data->currentGen, 
                            data->nextGen, std::array<short, 4>{x, y, 
                            static_cast<short>(x + x_part - 1), static_cast<short>(y + y_part - 1)},
                            id++));
                    }
                }

                // Display
                setlocale(LC_ALL, "");
                for (int x = 0; x < std::get<0>(coords); x++) {
                    for (int y = 0; y < std::get<1>(coords); y++) {
                        mvwprintw(board, x + 1, y + 1, "%s", 
                            (*data->currentGen.get())(x, y) ? "@" : " ");
                    }
                }

                data->tick++;
            }
            else currentTasksAmount = data->tasks.size();

            sync->queueInUse = false;
        }

        // Refresh display data
        mvwprintw(stats, 2, 4, "%s %llu", "Tick: ", data->tick);
        mvwprintw(stats, 3, 13, "%s", "   ");
        mvwprintw(stats, 3, 4, "%s %zu", "Workers: ", workers.size());
        mvwprintw(stats, 4, 11, "%s", "   ");
        mvwprintw(stats, 4, 4, "%s %d", "Tasks: ", tasksAmount);
        // mvwprintw(stats, 5, 4, "%s %d", "Current tasks: ", currentTasksAmount);
        mvwprintw(stats, 6, 12, "%s", "   ");
        mvwprintw(stats, 6, 4, "%s %d", "x_part: ", x_part);
        mvwprintw(stats, 7, 12, "%s", "   ");
        mvwprintw(stats, 7, 4, "%s %d", "y_part: ", y_part);
        mvwprintw(stats, 8, 4, "%s %d", "Pause: ", int(sync->pauseThreads));

        uniqueLock.unlock();
        sync->cvTasks.notify_all();

        wrefresh(board);
        wrefresh(stats);
    }

    sync->stopThreads = true;
    sync->pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    endwin();
}

void Game::addWorker() {
    if (workers.capacity() == workers.size())
        return;

    sync->workDone.push_back(0);
    workers.push_back(Worker(workers.size(), data, sync));
    workers.back().detach();
}

void Game::deleteWorker() {
    if (workers.size() <= 1)
        return;

    // Is it even safe bro? Does it even do it's job lmao?
    workers.pop_back();
    sync->workDone.pop_back();
}

std::tuple<int, short, short> Game::getTasksSize() {
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    short x_part = std::get<0>(data->currentGen->dimensions());
    short y_part = std::get<1>(data->currentGen->dimensions());

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

    return std::make_tuple(tasksAmount, x_part, y_part);
}

void Game::inputFunction(std::atomic_char& a_char) {
    while (!sync->stopThreads)
        a_char = getch();
}