#pragma once

#include <functional>
#include <chrono>

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/threads/pool.hpp>

#include "../control/controller.h"

class NewGameForm : public nana::form {
	nana::textbox text_width{ *this, "Width" };
	nana::textbox text_height{ *this, "Height" };
	nana::textbox text_num_mines{ *this, "# Mines" };
	nana::button btn_start{ *this, "Start" };
	nana::button btn_cancel{ *this, "Cancel" };

	using StartCallback = std::function<void(int, int, int)>;
	StartCallback start_cb;

	void place_components() {
		nana::place place{ *this };
		place.div("vert<textwidth><textheight><textmines><cancel><start>");

		place["textwidth"] << text_width;
		place["textheight"] << text_height;
		place["textmines"] << text_num_mines;
		place["cancel"] << btn_cancel;
		place["start"] << btn_start;

		place.collocate();
	}

	void new_game() {
		std::cout << "width: " << text_width.text() << "\n";
		start_cb(std::stoi(text_width.text()),
			std::stoi(text_height.text()),
			std::stoi(text_num_mines.text()));
		close();
	}

public:
	NewGameForm(StartCallback start_cb)
		: text_width{ *this, "Width" }
		, text_height{ *this, "Height" }
		, text_num_mines{ *this, "# Mines" }
		, btn_start{ *this, "Start" }
		, btn_cancel{ *this, "Cancel" }
		, start_cb{ start_cb }
	{
		place_components();

		btn_cancel.events().click([this]() {
			this->close();
			});

		btn_start.events().click([this]() {
			new_game();
			});

		show();
	}
};

class Gui {
	static constexpr unsigned int cell_width_pixels = 20;
	static constexpr unsigned int menubar_height = 26;
	static constexpr unsigned int status_line_height = 20;

	nana::form form;
	nana::menubar menubar;
	std::vector<std::unique_ptr<nana::button>> mine_buttons;
	nana::label status_line;
	std::unique_ptr<NewGameForm> new_game_form;

	std::shared_ptr<Controller> control;
	nana::threads::pool thread_pool;
	int width;
	int height;
	int num_mines;

	void show_new_game_dialog() {
		new_game_form = std::make_unique<NewGameForm>([this](int w, int h, int num_mines) {
			this->num_mines = num_mines;
			create_buttons();
			place_components();
			control->new_game(w, h, num_mines);
			});
		new_game_form->events().unload([this]() {
			new_game_form = nullptr;
			});
	}

	void fill_menu_bar() {
		nana::menu& game_item = menubar.push_back("&Game");
		game_item.append("Reset", [this](nana::menu::item_proxy&) {
			control->new_game(width, height, num_mines);
			});
		game_item.append("New game", [this](nana::menu::item_proxy&) {
			this->show_new_game_dialog();
			});

		nana::menu& solver_item = menubar.push_back("&Solver");
		solver_item.append("One move", [this](nana::menu::item_proxy&) {
			control->auto_next_move();
			});
		solver_item.append("Mark bombs", [this](nana::menu::item_proxy&) {
			control->mark_bombs();
			});
		solver_item.append("Autoplay", [this](nana::menu::item_proxy&) {
			nana::threads::pool_push(thread_pool, [this]() {
				using namespace std::chrono_literals;
				control->auto_play(0ms);
				})();
			});
		solver_item.append("Autoplay with delay", [this](nana::menu::item_proxy&) {
			nana::threads::pool_push(thread_pool, [this]() {
				using namespace std::chrono_literals;
				control->auto_play(80ms);
				})();
			});
	}

	void create_buttons() {
		mine_buttons.clear();
		for (int x = 0, y = 0; y < height;) {
			mine_buttons.emplace_back(std::make_unique<nana::button>(form, " "));
			mine_buttons.back()->events().mouse_up([this, x, y](nana::arg_mouse const& arg) {
				if (arg.button == nana::mouse::left_button) {
					control->expose({ x, y });
				}
				else if (arg.button == nana::mouse::right_button) {
					control->toggle_marked_bomb(x, y);
				}
				});
			y = x < width - 1 ? y : y + 1;
			x = x < width - 1 ? x + 1 : 0;
		}
	}

	void place_components() {
		form.size(nana::size(
			width * cell_width_pixels,
			height * cell_width_pixels + menubar_height + status_line_height));

		//Layout management
		nana::place place = nana::place{ form };
		place.div("vert<menubar weight=" + std::to_string(menubar_height) + ">"
			"<minefield "
			"grid=[" + std::to_string(width) + "," + std::to_string(height) + "]>"
			"vert<statusline weight=" + std::to_string(status_line_height) + ">"
		);
		place["menubar"] << menubar;
		for (auto const& cell : mine_buttons) {
			place["minefield"] << *cell;
		}
		place["statusline"] << status_line;
		place.collocate();
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
		fill_menu_bar();

		create_buttons();

		place_components();

		control->set_update_view_callback([this](Minefield const& mf) {
			this->show_minefield(mf);
			});
	}

	void start() {
		form.show();

		//Start to event loop process, it blocks until the form is closed.
		nana::exec();
	}

	// Callback used when the minefield has updated, and we need to show the new state
	void show_minefield(Minefield const& minefield) {
		if (minefield.get_width() != width ||
			minefield.get_height() != height) {
			// field size has changed, so we need to redraw it
			width = minefield.get_width();
			height = minefield.get_height();
			create_buttons();
			place_components();
		}

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