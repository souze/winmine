
#include "../model/minefield.h"
#include "../control/controller.h"
#include "solver_helpers.h"
#include "solver.h"

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/threads/pool.hpp>

#include <vector>

int main() {
	int const field_width = 4;
	int const field_height = 4;
	int const num_mines = 5;
	
	nana::form form;
	nana::button start_button{ form, "start" };
	std::vector<std::unique_ptr<nana::label>> labels;
	nana::label status_line{ form, "status" };

	// fill form with labels
	for (size_t x = 0, y = 0; y < field_height;) {
		labels.emplace_back(std::make_unique<nana::label>(form, "o"));
		y = x < field_width - 1 ? y : y + 1;
		x = x < field_width - 1 ? x + 1 : 0;
	}

	//Layout management
	form.div("vert<startbutton weight=28><minefield "
		"grid=[" + std::to_string(field_width) + "," + std::to_string(field_height) + "]>"
		"vert<statusline weight=28>"
	);
	form["startbutton"] << start_button;
	for (std::unique_ptr<nana::label> const& cell : labels) {
		form["minefield"] << *cell;
	}
	form["statusline"] << status_line;
	form.collocate();

	std::shared_ptr<Controller> control = std::make_shared<Controller>(
		Minefield{ field_width, field_height, std::vector<util::Pos>{ {0,1}, {1,1}, {2,1} } }
	);

	control->expose({ 0, 0 });
	nana::threads::pool thread_pool;
	start_button.events().click(nana::threads::pool_push(thread_pool, [&control]() {
		std::cout << "does it even work?\n";
		Solver::find_best_moves(control->get_minefield());
		}));

	form.show();

	register_debug_callback(
		[&](std::string const& msg, SolverField const& solverfield, Minefield const& minefield) {
		
		status_line.caption(msg);
		for (int i = 0; i < field_height*field_width; ++i) {
			int x = i % field_width;
			int y = i / field_width;

			Square const& sq = solverfield.squares[i];
			labels[i]->caption(
				std::to_string(sq.bomb_count) + "\n" +
				(sq.is_visited ? "v" : "") +
				(sq.is_bomb ? ", b" : "")
			);
		}

		});

	//Start to event loop process, it blocks until the form is closed.
	nana::exec();
}