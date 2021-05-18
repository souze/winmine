#include "solver.h"

#include "../control/controller.h"
//#include "../model/minefield.h"

namespace Solver {

void make_one_move(Controller& control) {
	control.expose(0, 0);
}

} // namespace Solver