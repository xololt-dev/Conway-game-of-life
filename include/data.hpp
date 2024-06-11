#pragma once

#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

#include "matrix.hpp"
#include "task.hpp"

enum ObjectType {
    point = 0,
    glider = 1,
    lwss = 2,
    hwss = 3,
    toad = 4,
    beacon = 5,
    penta_decatlon = 6,
    block = 7,
    beehive = 8,
    loaf = 9
};

struct Data {
    std::string pathToFile;

    unsigned long long tick = 0;
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    std::queue<Task> tasks;

    std::tuple<short, short> cursorPlacement;
    std::vector<std::tuple<short, short>> paintPoints;
    ObjectType placementType = point;

    Data() {}

    Data(Data &a_data) {
        tick = a_data.tick;
        currentGen = a_data.currentGen;
        nextGen = a_data.nextGen;
        tasks = a_data.tasks;
    }

    std::string getTypeString();

    bool loadData();

    void recalcPlacement(const short a_yMoveAxis = 0, const short a_xMoveAxis = 0);
};