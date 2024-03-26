#pragma once

#include "matrix.hpp"

#include <iostream>
#include <memory>
#include <tuple>

class Game {
public:
    void print() {
        std::tuple<short, short> dims = currentGen->dimensions();
        std::cout << "Yo! " << std::get<0>(dims) << " " << std::get<1>(dims) << std::endl;
    }

    Game(short const a_rows) {
        currentGen = std::make_unique<Matrix<short>>(a_rows, a_rows);
        nextGen = std::make_unique<Matrix<short>>(a_rows, a_rows);
    }

    Game(short const a_rows, short const a_columns) {
        currentGen = std::make_unique<Matrix<short>>(a_rows, a_columns);
        nextGen = std::make_unique<Matrix<short>>(a_rows, a_columns);
    }

private:
    std::unique_ptr<Matrix<short>> currentGen,
                                   nextGen;
    unsigned long long tick = 0;

    // Other threads
};