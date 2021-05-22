#pragma once

#include <functional>
#include <chrono>
#include <thread>

#include "../model/minefield.h"
#include "../solver/solver.h"
#include "../lib/util.h"

class Controller {
	Minefield minefield;
	std::function<void(Minefield const&)> update_view_callback;

	void update_view() {
		if (update_view_callback) {
			update_view_callback(minefield);
		}
	}

public:
	Controller(Minefield&& m)
		: minefield{std::move(m)}
	{}

	void expose(util::Pos pos) {
		std::cout << "Exposing " << pos << '\n';
		minefield.expose(pos.x, pos.y);
		update_view();
	}

	void toggle_marked_bomb(int x, int y) {
		minefield.toggle_marked_bomb(x, y);
		update_view();
	}

	void new_game(int width, int height, int num_mines) {
		minefield = Minefield{ width, height, num_mines };
		update_view();
	}

	void auto_next_move() {
		auto const& moves = Solver::find_best_moves(minefield);
		if (!moves.empty()) {
			expose(moves.back());
		}
		else {
			expose({ rand() % minefield.get_width(), rand() % minefield.get_height() });
		}
	}

	void auto_play(std::chrono::milliseconds delay) {
		while (!minefield.is_game_lost() && !minefield.is_game_won()) {
			Solver::board_state_result result = Solver::explore_possible_minefield_states(minefield);
			if (!result.safest_positions.empty()) {
				expose(result.safest_positions.back());
			}
			else {
				expose({ rand() % minefield.get_width(), rand() % minefield.get_height() });
			}
			if (result.unsafe_certainty > .99) {
				mark_as_mines(result.unsafest_positions);
			}
			std::this_thread::sleep_for(delay);
		}
	}

	void mark_as_mines(std::vector<util::Pos> const& positions) {
		for (util::Pos pos : positions) {
			Cell const& cell = minefield.get_cell(pos);
			if (!cell.is_marked_bomb()) {
				minefield.toggle_marked_bomb(pos.x, pos.y);
			}
		}
	}

	void mark_bombs() {
		mark_as_mines(Solver::find_bombs(minefield));
		update_view();
	}

	void set_update_view_callback(std::function<void(Minefield const&)> cb) {
		std::cout << "Setting update view CB\n";
		update_view_callback = cb;
	}

	Minefield const& get_minefield() {
		return minefield;
	}

	Controller(Controller&) = delete;
};