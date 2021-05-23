#pragma once

#include <vector>
#include <iostream>

namespace util {

struct Pos {
    int x = -1; // to detect uninitialized
    int y = -1; // to detect uninitialized

    Pos() = default;
    Pos(int x, int y)
        : x(x)
        , y(y)
    {}

    friend bool operator==(Pos const& lhs, Pos const& rhs);
    friend std::ostream& operator<<(std::ostream& os, Pos const& pos);
};

struct GameSettings {
    int width;
    int height;
    int num_bombs;
};

// Returns up to 8 adjacent positions. Less if the input position is next to a wall.
std::vector<Pos> get_adjacent_positions(Pos pos, int max_width, int max_height);

} // namespace util