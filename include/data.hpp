#pragma once

#include <fstream>
#include <ios>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <vector>

#include "matrix.hpp"
#include "task.hpp"

struct Data {
    unsigned long long tick = 0;
    std::string pathToFile;
    std::shared_ptr<Matrix<short>> currentGen,
                                   nextGen;
    std::queue<Task> tasks;

    Data() {}

    Data(Data &a_data) {
        tick = a_data.tick;
        currentGen = a_data.currentGen;
        nextGen = a_data.nextGen;
        tasks = a_data.tasks;
    }

    bool loadData() {
        std::fstream file;
        file.open(pathToFile, std::ios::in);

        if (!file.good())
            return false;

        int in = 0;
        int row, column;
        std::vector<short> temp;
        if (file.is_open()) {
            file >> row;
            file >> column;

            currentGen = std::make_shared<Matrix<short>>(row, column);
            nextGen = std::make_shared<Matrix<short>>(row, column);

            while (!file.eof()) {
                file >> in;
                temp.push_back(in);
            }

            currentGen->load(temp);
            
            return true;
        }
        else {
            (*this->currentGen.get())(2, 0) = 1;
            (*this->currentGen.get())(2, 1) = 1;
            (*this->currentGen.get())(2, 2) = 1;

            return false;
        }
    }
};