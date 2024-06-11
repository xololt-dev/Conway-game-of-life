#pragma once

#include <array>

struct Task {
    Task () {}
    Task(std::array<short, 4> a_coords) :
        coordinates(a_coords) { }

    std::array<short, 4> coordinates;
};