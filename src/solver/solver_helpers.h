#pragma once

#include <functional>

#include "../lib/util.h"
#include "../model/minefield.h"

struct Square {
	util::Pos pos;
	bool is_visited = false;
	bool is_bomb = false;
	int bomb_count = 0;

	Square(util::Pos p)
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

	std::vector<Square> squares{};
	int max_width = -1;
	int max_height = -1;
	int placed_bombs = 0;

	SolverField(int width, int height)
		: max_width(width)
		, max_height(height)
	{
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				squares.emplace_back(util::Pos{ x, y });
			}
		}
	}

	std::vector<Square*> get_adjacent_unexposed_squares(util::Pos const& sq, Minefield const minefield) {
		std::vector<Square*> adj_squares;

		std::vector<std::tuple<int, int>> adj_coords = util::get_adjacent_indices(sq.x, sq.y, max_width, max_height);

		for (std::tuple<int, int> const& coord : adj_coords) {
			Square& sq = squares[std::get<1>(coord) * max_width + std::get<0>(coord)];
			if (!minefield.get_cell(sq.pos).is_exposed()) {
				adj_squares.push_back(&sq);
			}
		}

		return adj_squares;
	}

	int count_adjacent_bombs(util::Pos pos, Minefield const& minefield) {

		std::vector<Square*> const adj_squares = get_adjacent_unexposed_squares(pos, minefield);

		return std::count_if(adj_squares.begin(), adj_squares.end(), [](Square const* square) {
			return square->is_bomb;
			});

	}

};

using debug_function_type = std::function<void(std::string const&, SolverField const&, Minefield const&)>;

void register_debug_callback(debug_function_type cb);
void solver_debug(std::string const& message, SolverField const& solverfield, Minefield const&);