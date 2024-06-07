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
    // Input thread
    std::atomic_char input(' ');
    std::thread inputThread = std::thread(&Game::inputFunction, this, std::ref(input));
    inputThread.detach();

    loadGeneration(std::ref(input));

    // Setup
    sync->pauseThreads = true;

    // Detaching thread(s)
    sync->workDone.push_back(0);
    workers.push_back(Worker(workers.size(), data, sync));    
    workers.back().detach();
    
    // Task creation
    std::tuple<short, short> coords = data->currentGen->dimensions();
    short x_part = std::get<0>(coords);
    short y_part = std::get<1>(coords);

    createNewTasks(coords, x_part, y_part);

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bool addThread = false;
    bool deleteThread = false;

    int currentTasksAmount = 0;
    double lastIteration = 0.0;
    int workersAmount = workers.size();
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    graphics->setWorkers(workersAmount);
    graphics->setTasks(tasksAmount);
    graphics->setXPart(x_part);
    graphics->setYPart(y_part);
    //graphics->setPaused(sync->pauseThreads);
    graphics->setTimer(lastIteration);

    graphics->fullGameRender();

    // Loop
    while (true) {
        sync->pauseThreads = false;
        sync->cvTasks.notify_one();

        switch (input) {
            // Quit
            case 'q':
                goto exit_loop;
            
            // Add thread
            case '+':
            case '=':
                if (deleteThread)
                    deleteThread = false;
                else
                    addThread = true;

                input = ' ';
                break;
            
            // Delete thread
            case '-':
                if (addThread)
                    addThread = false;
                else
                    deleteThread = true;
                
                input = ' ';
            
            // Pause threads
            case 'p':
                sync->pauseThreads = true;
                break;

            // Resume threads
            case 'r':
                sync->pauseThreads = false;
                break;

            default:
                break; 
        }

        /*
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
        // Pause threads
        else if (input == 'p') {
            sync->pauseThreads = true;
        }
        // Resume threads
        else if (input == 'r') {
            sync->pauseThreads = false;
            input = ' ';
        }
        */

        std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);
        sync->noTasks.wait(uniqueLock);

        if (!sync->queueInUse) {
            sync->queueInUse = true;

            bool canContinue = false;
            for (short& done : sync->workDone) {
                if (done == true) 
                    canContinue = true;
                
                else {
                    canContinue = false;
                    break;
                }
            }

            if (data->tasks.empty() && canContinue && !sync->pauseThreads) {
                sync->pauseThreads = true;

                if (deleteThread) {
                    deleteWorker();

                    std::tuple<int, short, short> tempTuple = getTasksSize();
                    tasksAmount = std::get<0>(tempTuple);
                    x_part = std::get<1>(tempTuple);
                    y_part = std::get<2>(tempTuple);

                    workersAmount = workers.size();
                    
                    graphics->renderLesserStats();
                }
                else if (addThread) {
                    addWorker();

                    std::tuple<int, short, short> tempTuple = getTasksSize();
                    tasksAmount = std::get<0>(tempTuple);
                    x_part = std::get<1>(tempTuple);
                    y_part = std::get<2>(tempTuple);
                    
                    workersAmount = workers.size();
                    
                    graphics->renderLesserStats();
                }
                deleteThread = false;
                addThread = false;

                lastIteration = double(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());

                start = std::chrono::system_clock::now();

                // Swap 
                data->currentGen->load(data->nextGen->board());

                // Assign new tasks
                createNewTasks(coords, x_part, y_part);

                // Display
                graphics->renderBoard();

                data->tick++;
            }
            else currentTasksAmount = data->tasks.size();

            sync->queueInUse = false;
        }

        // Refresh display data
        graphics->renderStats();
    }

    exit_loop:

    sync->stopThreads = true;
    sync->pauseThreads = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

void Game::createNewTasks(const std::tuple<short, short>& a_coords, const short& a_xPart, const short& a_yPart) {
    unsigned short id = 0;

    for (short x = 0; x < std::get<0>(a_coords); x += a_xPart) {
        for (short y = 0; y < std::get<1>(a_coords); y += a_yPart) {
            data->tasks.push(Task(data->currentGen, 
                data->nextGen, std::array<short, 4>{x, y, 
                static_cast<short>(x + a_xPart - 1), static_cast<short>(y + a_yPart - 1)},
                id++));
        }
    }
}

void Game::inputFunction(std::atomic_char& a_char) {
    while (!sync->stopThreads)
        a_char = getch();
}

void Game::loadGeneration(std::atomic_char& a_input) {
    graphics->setData(data);
    graphics->clearAll();
    bool loadedData = false;
    
    while (!loadedData) {
        graphics->renderLoadBoard();

        switch (a_input) {
            // Quit
            case char(27):
                (*data->currentGen.get())(3, 0) = 1;
                (*data->currentGen.get())(3, 1) = 1;
                (*data->currentGen.get())(3, 2) = 1;
                loadedData = true;
                break;

            // Backspace
            case char(8):
            case char(127):
                if (data->pathToFile.length())
                    data->pathToFile.pop_back();
                
                break;

            // Enter
            case '\n':
            case '\r':
                loadedData = true;
                if (!data->loadData())
                    return;
                break;

            default:
                if (a_input >= 32 and a_input < 127) {
                    if (data->pathToFile != "") 
                        data->pathToFile.push_back(a_input);
                    else data->pathToFile = a_input;
                }

                break;
        }

        a_input = char(128);
    }

    graphics->clearAll();
    a_input = ' ';
    graphics->setCoords(data->currentGen->dimensions());
}