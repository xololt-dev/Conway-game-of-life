#pragma once

#include "matrix.hpp"

#include <memory>
#include <tuple>

class Task {
public:
    Task(std::shared_ptr<Matrix<short>> a_current, 
        std::shared_ptr<Matrix<short>> a_next, 
        std::tuple<short, short, short, short> a_coords) :
        currentGen(a_current), nextGen(a_next), coordinates(a_coords) 
        { }

// private:
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    std::tuple<short, short, short, short> coordinates;
};