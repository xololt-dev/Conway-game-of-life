#pragma once

#include <memory>
#include <queue>

#include "matrix.hpp"
#include "task.hpp"

struct Data {
    unsigned long long tick = 0;
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    std::queue<Task> tasks;
};