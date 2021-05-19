#pragma once

#include <functional>

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/menubar.hpp>

#include "../control/controller.h"

class Gui {
	nana::form form;

	std::shared_ptr<Controller> control;
	std::vector<std::unique_ptr<nana::button>> mine_buttons;
	nana::menubar menubar;
	nana::label status_line;
	int width;
	int height;
	int num_mines;

	void fill_menu_bar() {
		nana::menu& game_item = menubar.push_back("&Game");
		game_item.append("Reset", [this](nana::menu::item_proxy&) {
			control->new_game(width, height, num_mines);
			});
		game_item.append("Settings", [](nana::menu::item_proxy&) {});

		nana::menu& solver_item = menubar.push_back("&Solver");
		solver_item.append("One move", [this](nana::menu::item_proxy&) {
			control->auto_next_move();
			});
		solver_item.append("Mark bombs", [](nana::menu::item_proxy&) {});
		solver_item.append("Autoplay", [](nana::menu::item_proxy&) {});
		solver_item.append("Autoplay with delay", [](nana::menu::item_proxy&) {});
	}

public:
	Gui(int field_width, int field_height, int num_mines, std::shared_ptr<Controller> control) 
		: width{ field_width }
		, height{ field_height }
		, num_mines{ num_mines }
		, control{ control }
		, form{}
		, menubar{ form }
		, status_line{ form }
	{
		// fill form with buttons
		for (size_t x = 0, y = 0; y < field_height;) {
			mine_buttons.emplace_back(std::make_unique<nana::button>(form, " "));
			mine_buttons.back()->events().mouse_up([control, x, y](nana::arg_mouse const& arg) {
				if (arg.button == nana::mouse::left_button) {
					control->expose(x, y);
				}
				else if (arg.button == nana::mouse::right_button) {
					control->toggle_marked_bomb(x, y);
				}
				});
			y = x < field_width - 1 ? y : y + 1;
			x = x < field_width - 1 ? x + 1 : 0;
			std::cout << x << ", " << y << '\n';
		}

		fill_menu_bar();

		//Layout management
		nana::place place = nana::place{ form };
		place.div("vert<menubar weight=28><minefield "
			"grid=[" + std::to_string(field_width) + "," + std::to_string(field_height) + "]>"
			"vert<statusline weight=28>"
		);
		place["menubar"] << menubar;
		for (auto const& cell : mine_buttons) {
			place["minefield"] << *cell;
		}
		place["statusline"] << status_line;
		place.collocate();

		// TODO must be a better way to do this, instead of bind+mem_fn
		control->set_update_view_callback([this](Minefield const& mf) {
			std::cout << "Update view CB\n";
			this->show_minefield(mf);
			});
	}

	void start() {
		form.show();

		//Start to event loop process, it blocks until the form is closed.
		nana::exec();
	}

	void show_minefield(Minefield const& minefield) {
		for (auto const& [x, y, cell] : minefield) {
			nana::button& btn = *mine_buttons[y * width + x];
			if (cell.is_exposed()) {
				if (cell.is_mine()) {
					btn.caption("x");
				}
				else if (cell.num_adjacent_mines() > 0) {
					btn.caption(std::to_string(cell.num_adjacent_mines()));
				}
				else {
					btn.caption("_"); // pressed but no adjacent mines
				}
			}
			else if (cell.is_marked_bomb()) {
				btn.caption("B");
			}
			else {
				btn.caption(" ");
			}
		}
		if (minefield.is_game_lost()) {
			status_line.caption("You lost");
		}
		else if (minefield.is_game_won()) {
			status_line.caption("You won");
		}
		else {
			status_line.caption("");
		}
	}

	Gui(Gui&) = delete;
};