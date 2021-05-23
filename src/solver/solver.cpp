#include "solver.h"

#include "solver_helpers.h"

#include "../control/controller.h"
#include "../model/minefield.h"
#include "../lib/util.h"

#include <algorithm>
#include <set>
#include <string>

using util::Pos;

namespace { // Anonymous namespace

void print_debug_solverfield(SolverField const& solverfield) {

    std::cout << "--------";
    for (int i = 0; i < solverfield.squares.size(); ++i) {
        if (i % solverfield.max_width == 0) {
            std::cout << '\n';
        }
        std::cout << "    " << std::to_string(solverfield.squares[i].bomb_count);
    }
    std::cout << '\n';

}

/*
* Mark squares in input list as visited. Return list of squares that where not visited before.
*/
std::vector<Square*> mark_squares_visited(std::vector<Square*> const& squares) {
    std::vector<Square*> newly_visited;
    for (Square* sq : squares) {
        if (!sq->is_visited) {
            newly_visited.push_back(sq);
        }
        sq->is_visited = true;
    }
    return newly_visited;
}

void unmark_squares_visited(std::vector<Square*> const& squares) {
    std::for_each(squares.begin(), squares.end(), [](Square* sq) {
        sq->is_visited = false;
        });
}

int count_bombs(std::vector<Square*> const& squares) {
    return std::count_if(squares.begin(), squares.end(), [](Square const* square) {
        return square->is_bomb;
        });
}

/*
* visit the square at position at the to of the exposed_squares input.
* For that position, satisfy the bomb-count by placing bombs on adjacent uncovered squares, in every possible permutation.
* When the bomb-count is satisfied, pop the current square from the exposed_squares stack, and recursively move on to the next.
* When the exposed_squares stack is empty, we have found a possible "solution", one variation of how bombs can be placed to
* satisfy all the conditions.
* When we have found a working solution, add 1 to all the squares that where marked as bombs in that solution.
*/
int count_possible_bomb_locations(SolverField& solverfield, std::vector<Pos> exposed_squares, Minefield const& minefield) {

    // We have evaluated all exposed squares correctly, this is a valid solution
    if (exposed_squares.empty()) {
        return 1;
    }

    Pos pos = exposed_squares.back();
    std::vector<Square*> adjacent_squares = solverfield.get_adjacent_covered_squares(pos, minefield);
    Cell const& actual_cell = minefield.get_cell(pos);

    auto goto_next_square = [
        &exposed_squares,
            &adjacent_squares,
            &solverfield,
            &minefield,
            pos]() {
        exposed_squares.pop_back();
        std::vector<Square*> marked_squares = mark_squares_visited(adjacent_squares);

        int tot_num_solutions = count_possible_bomb_locations(solverfield, exposed_squares, minefield);

        unmark_squares_visited(marked_squares);
        exposed_squares.push_back(pos);

        return tot_num_solutions;
    };

    int num_bombs_to_place = actual_cell.get_num_adjacent_bombs() - count_bombs(adjacent_squares);

    if (num_bombs_to_place < 0) {
        return 0; // Too many adjacent bombs, this is not a valid solution
    }
    else if (num_bombs_to_place == 0) { // Criterion already satisfied, just go on with the list
        return goto_next_square();
    }
    else { // Need to place 1 or more bombs for the criterion to be satisfied
        if (solverfield.placed_bombs == minefield.get_num_mines()) {
            // Can't place more, already at quota, this is not a solution
            return 0;
        }

        int tot_num_solutions = 0;
        for (Square* adj_sq_ptr : adjacent_squares) {
            Square& adj_sq = *adj_sq_ptr;
            if (!adj_sq.is_visited
                && !adj_sq.is_bomb
                && !minefield.get_cell(adj_sq.pos).is_exposed()) {
                adj_sq.is_bomb = true;
                ++solverfield.placed_bombs;

                if (num_bombs_to_place == 1) { // This square is satisfied
                    int num_solutions = goto_next_square();
                    adj_sq.bomb_count += num_solutions;
                    tot_num_solutions += num_solutions;
                }
                else { // Not enough adjacent bombs for this square, need to place more
                    int num_solutions = count_possible_bomb_locations(solverfield, exposed_squares, minefield);
                    adj_sq.bomb_count += num_solutions;
                    tot_num_solutions += num_solutions;
                }

                adj_sq.is_bomb = false;
                --solverfield.placed_bombs;
            }
        }
        return tot_num_solutions;
    }

    return 0;
}

} // End anonymous namespace

namespace solver {

std::vector<Pos> find_best_moves(Minefield const& minefield) {
    return explore_possible_minefield_states(minefield).safest_positions;
}

std::vector<util::Pos> find_bombs(Minefield const& minefield) {
    board_state_result result = explore_possible_minefield_states(minefield);
    if (result.unsafe_certainty > .99) { // mark it as bomb if we are 99% certain
        return result.unsafest_positions;
    }
    return {};
}

/*
Explore all possible bomb placements, and return a structure with information about
best and worst possible moves.
*/
board_state_result explore_possible_minefield_states(Minefield const& minefield) {
    SolverField solverfield{ minefield.get_width(), minefield.get_height() };
    std::vector<Pos> exposed_squares;

    for (int y = 0; y < minefield.get_height(); ++y) {
        for (int x = 0; x < minefield.get_width(); ++x) {
            Pos pos{ x, y };
            if (minefield.get_cell(pos).is_exposed() && minefield.get_cell(pos).get_num_adjacent_bombs() > 0) {
                exposed_squares.push_back(pos);
            }
        }
    }

    int total_num_solutions = count_possible_bomb_locations(solverfield, exposed_squares, minefield);

    std::set<Square*> possible_bomb_squares; // only squares that are adjacent to exposed numbers are relevant
                                             // std::set to avoid duplicated squares.
    for (Pos const& pos : exposed_squares) {
        std::vector<Square*> adj_squares = solverfield.get_adjacent_covered_squares(pos, minefield);
        possible_bomb_squares.insert(adj_squares.begin(), adj_squares.end());
    }

    auto const [min, max] = std::minmax_element(possible_bomb_squares.begin(), possible_bomb_squares.end(),
        [](Square const* lhs, Square const* rhs) {
            return lhs->bomb_count < rhs->bomb_count;
        });

    std::vector<Pos> safe_squares, unsafe_squares;
    for (Square const* sq : possible_bomb_squares) {
        if (sq->bomb_count == (*min)->bomb_count) {
            safe_squares.push_back(sq->pos);
        }
        if (sq->bomb_count == (*max)->bomb_count) {
            unsafe_squares.push_back(sq->pos);
        }
    }

    double safe_certainty = possible_bomb_squares.empty() ?
        .5 :
        1 - (*min)->bomb_count / static_cast<double>(total_num_solutions);
    double unsafe_certainty = possible_bomb_squares.empty() ?
        .5 : (*max)->bomb_count / static_cast<double>(total_num_solutions);


    return board_state_result{
        /*.safest_positions = */safe_squares,
        /*.unsafest_positions = */unsafe_squares,
        /*.safe_certainty = */safe_certainty,
        /*.unsafe_certainty = */ unsafe_certainty
    };
}

} // namespace Solver