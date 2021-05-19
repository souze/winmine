#pragma once

#include <functional>

#include "../model/minefield.h"
#include "../solver/solver.h"

class Controller {
	Minefield minefield;
	std::function<void(Minefield const&)> update_view_callback;

public:
	Controller(Minefield&& m)
		: minefield{std::move(m)}
	{}

	void expose(int x, int y) {
		std::cout << "Expose on controller\n";
		minefield.expose(x, y);
		update_view_callback(minefield);
	}

	void toggle_marked_bomb(int x, int y) {
		minefield.toggle_marked_bomb(x, y);
		update_view_callback(minefield);
	}

	void new_game(int width, int height, int num_mines) {
		minefield = Minefield{ width, height, num_mines };
		update_view_callback(minefield);
	}

	void auto_next_move() {
		Solver::make_one_move(minefield, *this);
	}

	void auto_play() {
		// TODO solver::play(minefield);
	}

	void set_update_view_callback(std::function<void(Minefield const&)> cb) {
		std::cout << "Setting update view CB\n";
		update_view_callback = cb;
	}

	Controller(Controller&) = delete;
};