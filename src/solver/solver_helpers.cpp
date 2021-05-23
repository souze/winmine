#include "solver_helpers.h"

#include "../lib/util.h"

SolverField::SolverField(int width, int height)
    : max_width(width)
    , max_height(height)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            squares.emplace_back(util::Pos{ x, y });
        }
    }
}

std::vector<Square*> SolverField::get_adjacent_covered_squares(
        util::Pos pos, 
        Minefield const minefield) {
    std::vector<Square*> adj_squares;

    std::vector<util::Pos> adj_positions = util::get_adjacent_positions(pos, max_width, max_height);

    for (util::Pos p : adj_positions) {
        Square& sq = squares[p.y * max_width + p.x];
        if (minefield.get_cell(sq.pos).is_covered()) {
            adj_squares.push_back(&sq);
        }
    }

    return adj_squares;
}

