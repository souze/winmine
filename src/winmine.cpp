// winmine.cpp : Defines the entry point for the application.

#include "winmine.h"

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

#include "model/minefield.h"
#include "view/gui.h"
#include "control/controller.h"

struct ClickerCallback {
	Minefield* minefield;
	Gui* gui;

	ClickerCallback(Minefield& minefield)
		: minefield(&minefield)
	{}

	void operator()(int x, int y) {
		minefield->expose(x, y);
		gui->show_minefield(*minefield);
		std::cout << "(" << x << ", " << y << ")\n";
	}

	ClickerCallback(ClickerCallback&) = delete;
};

int main()
{
	int const field_height = 9;
	int const field_width = 10;
	int const num_mines = 11;

	std::shared_ptr<Controller> control = std::make_shared<Controller>(
			Minefield{ field_width, field_height, num_mines }
			);

	Gui gui = Gui{ field_width, field_height, control };

	gui.start();
}