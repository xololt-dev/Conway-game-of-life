#pragma once

#include "graphics.hpp"
#include "matrix.hpp"
#include "data.hpp"
#include "worker.hpp"
#include "sync.hpp"

#include <ncurses.h>
#include <atomic>
#include <memory>
#include <tuple>
#include <vector>
#include <thread>

class Game {
public:
    void start();
    void loadGeneration(std::atomic_char& a_input);

    void inputFunction(std::atomic_char& a_char);
    void addWorker();
    void deleteWorker();
    void placeObject();

    std::tuple<int, short, short> getTasksSize();
    void createNewTasks(const std::tuple<short, short>& a_coords, const short& a_xPart, const short& a_yPart);

    Game(short const a_rows) {
        data = std::make_shared<Data>();
        sync = std::make_shared<Sync>();
        graphics = std::make_shared<Graphics>(std::make_tuple(a_rows, a_rows));

        data->currentGen = std::make_shared<Matrix<short>>(a_rows, a_rows);
        data->nextGen = std::make_shared<Matrix<short>>(a_rows, a_rows);

        hintPreAllocate();
    }

    Game(short const a_rows, short const a_columns) {
        data = std::make_shared<Data>();
        sync = std::make_shared<Sync>();
        graphics = std::make_shared<Graphics>(std::make_tuple(a_rows, a_columns));

        data->currentGen = std::make_shared<Matrix<short>>(a_rows, a_columns);
        data->nextGen = std::make_shared<Matrix<short>>(a_rows, a_columns);

        hintPreAllocate();
    }

    void hintPreAllocate() {
        // Try to prealocate memory based on hardware hint
        unsigned int numThreads = std::thread::hardware_concurrency();

        if (numThreads)
            workers.reserve(numThreads);        
    }

private:
    std::shared_ptr<Data> data;
    std::shared_ptr<Sync> sync;
    std::shared_ptr<Graphics> graphics;

    std::vector<Worker> workers;
};