#include "util.h"

#include <vector>
#include <iostream>

namespace util {

bool operator==(Pos const& lhs, Pos const& rhs) {
    return lhs.x == rhs.x &&
        lhs.y == rhs.y;
}

std::ostream& operator<<(std::ostream& os, Pos const& pos) {
    os << "(" << pos.x << "," << pos.y << ")";
    return os;
}

// Returns up to 8 adjacent positions. Less if the input position is next to a wall.
std::vector<Pos> get_adjacent_positions(Pos pos, int max_width, int max_height) {
    std::vector<Pos> adjacent_positions;
    int const x = pos.x;
    int const y = pos.y;

    if (y - 1 >= 0) {
        if (x - 1 >= 0) adjacent_positions.emplace_back(x - 1, y - 1);
        adjacent_positions.emplace_back(x, y - 1);
        if (x + 1 < max_width) adjacent_positions.emplace_back(x + 1, y - 1);
    }
    if (x - 1 >= 0) adjacent_positions.emplace_back(x - 1, y);
    if (x + 1 < max_width) adjacent_positions.emplace_back(x + 1, y);
    if (y + 1 < max_height) {
        if (x - 1 >= 0) adjacent_positions.emplace_back(x - 1, y + 1);
        adjacent_positions.emplace_back(x, y + 1);
        if (x + 1 < max_width) adjacent_positions.emplace_back(x + 1, y + 1);
    }
    return adjacent_positions;
}

} // namespace util