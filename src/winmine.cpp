// winmine.cpp : Defines the entry point for the application.

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>

#include "model/minefield.h"
#include "view/gui.h"
#include "control/controller.h"


int main()
{
    util::GameSettings const settings{
        10 // height
        , 10 // width
        , 10 // num_bombs
    };

	std::shared_ptr<Controller> control = std::make_shared<Controller>(
		Minefield{ settings }
	);

	Gui gui = Gui{ settings, control };

	gui.start();
}