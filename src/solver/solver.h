#pragma once

#include "../lib/util.h"

// Forward declarations
class Controller;
class Minefield;

namespace Solver {

	std::vector<util::Pos> find_best_moves(Minefield const& minefield);

	util::Pos make_one_move(Minefield const& minefield, Controller& control);

} // namespace Solver