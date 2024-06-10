#include "data.hpp"

#include <tuple>
#include <fstream>
#include <ios>

std::string Data::getTypeString() {
    switch (placementType) {
        case point:
            return "point";

        case glider:
            return "glider";

        case lwss:
            return "lwss";

        case hwss:
            return "hwss";

        case toad:
            return "toad";

        case beacon:
            return "beacon";

        case penta_decatlon:
            return "penta_decatlon";

        case block:
            return "block";

        case beehive:
            return "beehive";

        case loaf:
            return "loaf";
    }    
}

bool Data::loadData() {
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

void Data::recalcPlacement(short a_yMoveAxis, short a_xMoveAxis) {
    // Wrap around
    if (a_yMoveAxis < 0 && std::get<0>(cursorPlacement) == 0) {
        std::get<0>(cursorPlacement) = std::get<0>(currentGen->dimensions()) - 1;
    }
    else if (a_yMoveAxis > 0 && std::get<0>(cursorPlacement) == std::get<0>(currentGen->dimensions()) - 1) {
        std::get<0>(cursorPlacement) = 0;
    }
    else if (a_xMoveAxis < 0 && std::get<1>(cursorPlacement) == 0) {
        std::get<1>(cursorPlacement) = std::get<1>(currentGen->dimensions()) - 1;
    }
    else if (a_xMoveAxis > 0 && std::get<1>(cursorPlacement) == std::get<1>(currentGen->dimensions()) - 1) {
        std::get<1>(cursorPlacement) = 0;
    }
    else {
        std::get<0>(cursorPlacement) += a_yMoveAxis;
        std::get<1>(cursorPlacement) += a_xMoveAxis;
    }

    paintPoints.clear();
    switch (placementType) {
        case point:
            paintPoints = {std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, std::get<1>(cursorPlacement) + 1)};
            break;

        case glider:
            paintPoints = {std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case lwss:
            paintPoints = {std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 4) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 4) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case hwss:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 4) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 6) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 6) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 4) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 4) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 5) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case toad:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case beacon:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case penta_decatlon:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 7) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 4) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 5) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 6) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 8) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 9) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 7) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case block:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case beehive:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1)};
            break;

        case loaf:
            paintPoints = {
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>(std::get<0>(cursorPlacement) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, std::get<1>(cursorPlacement) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 1) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 1) % std::get<1>(currentGen->dimensions()) + 1),
            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 2) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 3) % std::get<1>(currentGen->dimensions()) + 1),

            std::make_tuple<short, short>((std::get<0>(cursorPlacement) + 3) % std::get<0>(currentGen->dimensions()) + 1, (std::get<1>(cursorPlacement) + 2) % std::get<1>(currentGen->dimensions()) + 1)};
            break;
    }
}