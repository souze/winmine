#include "solver.h"

#include "../control/controller.h"
#include "../model/minefield.h"
#include "../lib/util.h"

#include <algorithm>

namespace { // Anonymous namespace

using util::Pos;

struct Square {
	Pos pos;
	bool is_visited = false;
	bool is_bomb = false;
	int bomb_count = 0;

	Square(Pos p)
		: pos(p)
	{}

	Square(Square&) = delete;

	Square(Square&& rhs) noexcept {
		pos = rhs.pos;
		is_visited = rhs.is_visited;
		is_bomb = rhs.is_bomb;
		bomb_count = rhs.bomb_count;
	}
};

struct SolverField {

	std::vector<Square> squares;
	int max_width = -1;
	int max_height = -1;

	SolverField(int width, int height) 
		: max_width(width)
		, max_height(height)
	{
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				squares.emplace_back(Pos{ x, y });
			}
		}
	}

	std::vector<Square*> get_adjacent_squares(Pos const& sq) {
		std::vector<Square*> adj_squares;

		std::vector<std::tuple<int, int>> adj_coords = util::get_adjacent_indices(sq.x, sq.y, max_width, max_height);
	
		for (std::tuple<int, int> const& coord : adj_coords) {
			adj_squares.push_back(&squares[std::get<1>(coord) * max_width + std::get<0>(coord)]);
		}

		return adj_squares;
	}

	int count_adjacent_bombs(Pos pos) {

		std::vector<Square*> const adj_squares = get_adjacent_squares(pos);

		return std::count_if(adj_squares.begin(), adj_squares.end(), [](Square const* square) {
			return square->is_bomb;
			});

	}

};


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
	
	if (exposed_squares.empty()) { // We have evaluated all exposed squares correctly, this is a valid solution
		return 1;
	}

	Pos sq = exposed_squares.back();

	std::vector<Square*> adjacent_squares = solverfield.get_adjacent_squares(sq);

	int num_bombs_to_place = minefield.get_cell(sq).num_adjacent_mines() - solverfield.count_adjacent_bombs(sq);
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
	else {
		int tot_num_solutions = 0;
		for (Square* adj_sq_ptr : adjacent_squares) {
			Square& adj_sq = *adj_sq_ptr;
			if (!adj_sq.is_visited
				&& !adj_sq.is_bomb
				&& !minefield.get_cell(adj_sq.pos).is_exposed()) {
				adj_sq.is_bomb = true;

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
					adj_sq.bomb_count += count_possible_bomb_locations(solverfield, exposed_squares, minefield);
				}

				adj_sq.is_bomb = false;
			}
		}
		return tot_num_solutions;
	}

	return 0;
}

void count_bomb_possible_locations(int width, int height, Minefield const& minefield) {
	SolverField solverfield{ width, height };
	std::vector<Pos> exposed_squares;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Pos pos{ x, y };
			if (minefield.get_cell(pos).is_exposed() && minefield.get_cell(pos).num_adjacent_mines() > 0) {
				exposed_squares.push_back(pos);
			}
		}
	}

	count_possible_bomb_locations(solverfield, exposed_squares, minefield);

	auto const [min, max] = std::minmax_element(solverfield.squares.begin(), solverfield.squares.end(),
		[](Square const& lhs, Square const& rhs) {
			return lhs.bomb_count < rhs.bomb_count;
		});

	std::vector<Square const*> safe_squares, unsafe_squares;
	for (Square const& sq : solverfield.squares) {
		if (sq.bomb_count == min->bomb_count) {
			safe_squares.push_back(&sq);
		}
		if (sq.bomb_count == max->bomb_count) {
			unsafe_squares.push_back(&sq);
		}
	}

	std::cout << "Safest squares";
	for (Square const* sq : safe_squares) {
		std::cout << ", (" << sq->pos.x << "," << sq->pos.y << ")";
	}
	
	std::cout << "\nUnsafest squares";
	for (Square const* sq : unsafe_squares) {
		std::cout << ", (" << sq->pos.x << "," << sq->pos.y << ")";
	}
	std::cout << "\n";
}

} // Anonymous namespace

namespace Solver {

void make_one_move(Minefield const& minefield, Controller& control) {
	count_bomb_possible_locations(minefield.get_width(), minefield.get_height(), minefield);
}

} // namespace Solver