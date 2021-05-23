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

// Theorethical minefield, used to gradually build up a possible mine permutation
struct SolverField {

	std::vector<Square> squares{};
	int max_width = -1;
	int max_height = -1;
	int placed_bombs = 0;

    SolverField(int width, int height);

    std::vector<Square*> get_adjacent_covered_squares(util::Pos pos, Minefield const minefield);
};
