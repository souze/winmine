// winmine.cpp : Defines the entry point for the application.

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

#include "model/minefield.h"
#include "view/gui.h"
#include "control/controller.h"


int main()
{
	int const field_height = 10;
	int const field_width = 10;
	int const num_mines = 10;

	std::shared_ptr<Controller> control = std::make_shared<Controller>(
			Minefield{ field_width, field_height, num_mines }
			);

	Gui gui = Gui{ field_width, field_height, num_mines, control };

	gui.start();
}