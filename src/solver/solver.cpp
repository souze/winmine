#include "solver.h"

#include "solver_helpers.h"

#include "../control/controller.h"
#include "../model/minefield.h"
#include "../lib/util.h"

#include <algorithm>
#include <set>
#include <string>

namespace { // Anonymous namespace

using util::Pos;

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

std::vector<Square*> mark_squares_visited(std::vector<Square*>& squares) {
	std::vector<Square*> newly_visited;
	for (Square* sq : squares) {
		if (!sq->is_visited) {
			newly_visited.push_back(sq);
		}
		sq->is_visited = true;
	}
	return newly_visited;
}

void unmark_squares(std::vector<Square*>& squares) {
	std::for_each(squares.begin(), squares.end(), [](Square* sq) {
		sq->is_visited = false;
		});
}

int count_possible_bomb_locations(SolverField& solverfield, std::vector<Pos> exposed_squares, Minefield const& minefield) {

	solver_debug("Entering reccursive call", solverfield, minefield);

	if (exposed_squares.empty()) { // We have evaluated all exposed squares correctly, this is a valid solution
		return 1;
	}

	Pos sq = exposed_squares.back();

	std::vector<Square*> adjacent_squares = solverfield.get_adjacent_unexposed_squares(sq, minefield);

	int num_bombs_to_place = minefield.get_cell(sq).num_adjacent_mines() - solverfield.count_adjacent_bombs(sq, minefield);
	if (num_bombs_to_place < 0) {
		return 0; // Too many adjacent bombs, this is not a valid solution
	}
	else if (num_bombs_to_place == 0) { // Criterion already satisfied, just go on with the list
		// TODO DRY
		exposed_squares.pop_back();

		std::vector<Square*> marked_squares = mark_squares_visited(adjacent_squares);

		int tot_num_solutions = count_possible_bomb_locations(solverfield, exposed_squares, minefield);

		unmark_squares(marked_squares);

		exposed_squares.push_back(sq);

		return tot_num_solutions;
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
					exposed_squares.pop_back();

					std::vector<Square*> marked_squares = mark_squares_visited(adjacent_squares);

					int num_solutions = count_possible_bomb_locations(solverfield, exposed_squares, minefield);
					adj_sq.bomb_count += num_solutions;
					tot_num_solutions += num_solutions;

					unmark_squares(marked_squares);

					exposed_squares.push_back(sq);
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

namespace Solver {

std::vector<Pos> find_best_moves(Minefield const& minefield) {
	SolverField solverfield{ minefield.get_width(), minefield.get_height() };
	std::vector<Pos> exposed_squares;

	for (int y = 0; y < minefield.get_height(); ++y) {
		for (int x = 0; x < minefield.get_width(); ++x) {
			Pos pos{ x, y };
			if (minefield.get_cell(pos).is_exposed() && minefield.get_cell(pos).num_adjacent_mines() > 0) {
				exposed_squares.push_back(pos);
			}
		}
	}

	count_possible_bomb_locations(solverfield, exposed_squares, minefield);

	std::set<Square*> possible_bomb_squares; // only squares that are adjacent to exposed numbers are relevant
											 // std::set to avoid duplicated squares.
	for (Pos const& pos : exposed_squares) {
		std::vector<Square*> adj_squares = solverfield.get_adjacent_unexposed_squares(pos, minefield);
		possible_bomb_squares.insert(adj_squares.begin(), adj_squares.end());
	}

	auto const [min, max] = std::minmax_element(possible_bomb_squares.begin(), possible_bomb_squares.end(),
		[](Square const* lhs, Square const* rhs) {
			return lhs->bomb_count < rhs->bomb_count;
		});

	std::vector<Square const*> safe_squares, unsafe_squares;
	for (Square const* sq : possible_bomb_squares) {
		if (sq->bomb_count == (*min)->bomb_count) {
			safe_squares.push_back(sq);
		}
		if (sq->bomb_count == (*max)->bomb_count) {
			unsafe_squares.push_back(sq);
		}
	}

	std::vector<Pos> safe_positions;
	std::transform(safe_squares.begin(), safe_squares.end(), std::back_inserter(safe_positions), [](Square const* sq) {
		return sq->pos;
		});
	return safe_positions;
}

Pos make_one_move(Minefield const& minefield, Controller& control) {
	return { -1, -1 }; // TODO is this function really needed?
}

} // namespace Solver