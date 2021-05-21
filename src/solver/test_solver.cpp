#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "solver.h"
#include "solver_helpers.h"
#include "../model/minefield.h"
#include "../control/controller.h"
#include "../lib/util.h"

#include <unordered_set>

using util::Pos;

namespace std {
template<> struct hash<Pos>
{
	std::size_t operator()(Pos const& pos) const noexcept
	{
		return pos.x ^ (pos.y << 1);
	}
};
}

std::vector<Pos> find_char_positions(char c, std::string const& board) {
	std::vector<Pos> positions;
	int x = 0;
	int y = 0;
	for (char ch : board) {
		if (ch == c) {
			positions.emplace_back(x, y);
		}
		if (ch == '\n') {
			if (x == 0 && y == 0) continue; // We ignore newline as the first character
			x = 0;
			++y;
		}
		else {
			++x;
		}
	}
	return positions;
}

std::unique_ptr<Controller> create_board(std::string const& board) {
	std::vector<Pos> bomb_positions = find_char_positions('b', board);

	int const width = board.length() - board.find_last_of('\n') - 1;
	int const height = std::count(board.begin(), board.end(), '\n');

	std::unique_ptr<Controller> control = std::make_unique<Controller>(
		Minefield{ width, height, bomb_positions }
	);

	for (Pos const& expose_pos : find_char_positions('o', board)) {
		control->expose(expose_pos);
	}

	return control;
}

template<typename T>
std::unordered_set<T> to_set(std::vector<T> const& v) {
	return std::unordered_set<T>{ v.cbegin(), v.cend() };
}

void test_moves_solver(std::string const& state_board, std::string const& expected_moves_board) {
	std::unique_ptr<Controller> control = create_board(state_board);

	std::vector<Pos> expected_moves = find_char_positions('m', expected_moves_board);

	std::vector<Pos> actual_moves = Solver::find_best_moves(control->get_minefield());

	REQUIRE(to_set(expected_moves) == to_set(actual_moves));
}

TEST_CASE("Basic 3x3", "[.]") {
	
	SECTION("Bomb in middle") {
		test_moves_solver(R"(
ooo
.b.
...)",

R"(
...
m.m
...)");

		test_moves_solver(R"(
o.o
.b.
...)",

R"(
.m.
mmm
...)");

		test_moves_solver(R"(
o..
.b.
...)",

R"(
.m.
mm.
...)");

		test_moves_solver(R"(
oo.
ob.
...)",

R"(
..m
..m
mm.)");

		test_moves_solver(R"(
ooo
.b.
o..)",

R"(
...
m.m
.m.)");

	}

	SECTION("Two bombs") {
		test_moves_solver(R"(
...
.o.
bb.)",

R"(
mmm
m.m
mmm)");
	}
}

TEST_CASE("Five x Five", "[5x5]") {

	test_moves_solver(R"(
....
.ob.
.bo.
....)",
R"(
mmm.
mo.m
m.om
.mmm)");

}