#include "game.hpp"
#include "data.hpp"
#include "task.hpp"
#include "worker.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
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

    // Setup
    loadGeneration(std::ref(input));
    sync->pauseThreads = true;

    // Task creation
    std::tuple<short, short> coords = data->currentGen->dimensions();
    short x_part = std::get<0>(coords);
    short y_part = std::get<1>(coords);

    createNewTasks(coords, x_part, y_part);

    // Detaching thread(s)
    sync->workDone.push_back(0);
    workers.push_back(Worker(workers.size(), data, sync));    
    workers.back().detach();

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    bool addThread = false;
    bool deleteThread = false;
    bool addObject = false;

    double lastIteration = 0.0;
    int workersAmount = workers.size();
    int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    graphics->setWorkers(workersAmount);
    graphics->setTasks(tasksAmount);
    graphics->setXPart(x_part);
    graphics->setYPart(y_part);
    graphics->setTimer(lastIteration);

    graphics->fullGameRender();
{
    std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);

    // Loop
    while (true) {
        sync->pauseThreads = false;
        sync->cvTasks.notify_one();

        if (handleInput(input, deleteThread, addThread, addObject))
            goto exit_loop;

        //std::unique_lock<std::mutex> uniqueLock(sync->tasksMutex);
        if (sync->noTasks.wait_for(uniqueLock, std::chrono::microseconds(10)) == std::cv_status::timeout)
            continue;

        if (!sync->pauseThreads) {
            // Is this needed? Assumed yes, might show up at bigger gen sizes
            bool canContinue = false;
            for (short& done : sync->workDone) {
                if (done) 
                    canContinue = true;
                
                else {
                    canContinue = false;
                    break;
                }
            }

            if (canContinue) {
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

                if (addObject) {
                    placeObject();
                    addObject = false;
                }

                lastIteration = double(std::chrono::duration<double>(std::chrono::system_clock::now() - start).count());

                start = std::chrono::system_clock::now();

                data->tick++;

                // Swap 
                data->currentGen->load(data->nextGen->board());

                // Assign new tasks
                createNewTasks(coords, x_part, y_part);

                // Display
                graphics->renderBoard();
            }
        }

        // Refresh display data
        graphics->renderStats();
    }
}
    exit_loop:;

    for (Worker& w : workers)
        w.failSafe = true;

    sync->pauseThreads = true;
    sync->stopThreads = true;
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

    workers.end()->failSafe = true;
    while (sync->workDone[workers.size()] == 1)
        continue;

    workers.pop_back();
    sync->workDone.pop_back();
}

void Game::placeObject() {
    Matrix<short>& matrix = *data->nextGen.get();
    
    for (std::tuple<short, short>& t : data->paintPoints)
        matrix(std::get<0>(t) - 1, std::get<1>(t) - 1) = 1;
}

std::tuple<int, short, short> Game::getTasksSize() {
    unsigned int tasksAmount = std::pow(2, std::ceil(log2(workers.size())));

    unsigned short x_part = std::get<0>(data->currentGen->dimensions());
    unsigned short y_part = std::get<1>(data->currentGen->dimensions());

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
            data->tasks.push(Task(std::array<short, 4>{x, y, 
                static_cast<short>(x + a_xPart - 1), static_cast<short>(y + a_yPart - 1)}));
        }
    }
}

void Game::inputFunction(std::atomic_char& a_char) {
    while (!sync->stopThreads)
        a_char = getch();
}

bool Game::handleInput(std::atomic_char& a_input, bool& a_deleteThread, bool& a_addThread, bool& a_addObject) {
    switch (a_input) {
        // Quit
        case 'q':
            return true;
        
        // Add thread
        case '+':
        case '=':
            if (a_deleteThread)
                a_deleteThread = false;
            else
                a_addThread = true;

            a_input = ' ';
            break;
        
        // Delete thread
        case '-':
            if (a_addThread)
                a_addThread = false;
            else
                a_deleteThread = true;
            
            a_input = ' ';
        
        // Pause threads
        case 'p':
            sync->pauseThreads = true;
            break;

        // Resume threads
        case 'r':
            sync->pauseThreads = false;
            break;

        // Move object
        case 'w':
        case 'a':
        case 's':
        case 'd': {
            short yMoveAxis = (a_input == 's') - (a_input == 'w');
            short xMoveAxis = (a_input == 'd') - (a_input == 'a');

            data->recalcPlacement(yMoveAxis, xMoveAxis);
        }
            a_input = ' ';

            break;

        // Insert 'something'
        case 'i':
            a_addObject = true;
            a_input = ' ';
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            data->placementType = ObjectType(int(a_input) - '0');
            data->recalcPlacement();
            a_input = ' ';
            break;

        default:
            break; 
    }

    return false;
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