#pragma once

#include "../lib/util.h"

// Forward declarations
class Controller;
class Minefield;

namespace solver {

struct board_state_result {
	std::vector<util::Pos> safest_positions;
	std::vector<util::Pos> unsafest_positions;

	double safe_certainty = .5;   // 0-100%, 0% means definitely a bomb, 100% means definitely safe
	double unsafe_certainty = .5;   // 0-100%, 0% means definitely safe, 100% means definitely a bomb
};

board_state_result explore_possible_minefield_states(Minefield const& minefield);

std::vector<util::Pos> find_best_moves(Minefield const& minefield);

std::vector<util::Pos> find_bombs(Minefield const& minefield);

} // namespace Solver