#pragma once

#include <functional>

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>

#include "../control/controller.h"

class Gui {
	nana::form form;

	std::shared_ptr<Controller> control;
	std::vector<std::unique_ptr<nana::button>> mine_buttons;
	int const width;
	int const height;

public:
	Gui(int field_width, int field_height, std::shared_ptr<Controller> control) 
		: width{ field_width }
		, height{ field_height }
		, control{ control }
	{
		// fill form with buttons
		for (size_t x = 0, y = 0; y < field_height;) {
			mine_buttons.emplace_back(std::make_unique<nana::button>(form, " "));
			mine_buttons.back()->events().mouse_up([control, x, y](nana::arg_mouse const& arg){
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

		//Layout management
		form.div("<minefield grid=[10,10]>");

		for (auto const& cell : mine_buttons) {
			form["minefield"] << *cell;
		}

		// TODO must be a better way to do this, instead of bind+mem_fn
		control->set_update_view_callback([this](Minefield const& mf) {
			std::cout << "Update view CB\n";
			this->show_minefield(mf);
			});
	}

	void start() {
		form.collocate();

		form.show();

		//Start to event loop process, it blocks until the form is closed.
		nana::exec();
	}

	void show_minefield(Minefield const& minefield) {
		for (auto const& [x, y, cell] : minefield) {
			nana::button& btn = *mine_buttons[y * width + x];
			if (cell.is_exposed()) {
				std::cout << "showing (" << x << ", " << y << ") as exposed\n";
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
				std::cout << "(" << x << ", " << y << ") not exposed\n";
			}
		}
	}

	Gui(Gui&) = delete;
};