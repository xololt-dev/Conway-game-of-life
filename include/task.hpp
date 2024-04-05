#pragma once

#include "matrix.hpp"

#include <memory>
#include <array>

class Task {
public:
    Task() {}

    Task(std::shared_ptr<Matrix<short>> a_current, 
        std::shared_ptr<Matrix<short>> a_next, 
        std::array<short, 4> a_coords,
        unsigned short a_id) :
        // std::tuple<short, short, short, short> a_coords) :
        currentGen(a_current), nextGen(a_next), coordinates(a_coords), id(a_id)
        { }

// private:
    unsigned short id = 0;
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    //std::tuple<short, short, short, short> 
    std::array<short, 4> coordinates;
};