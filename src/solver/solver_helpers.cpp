#include "solver_helpers.h"

static debug_function_type solver_debug_cb;

void register_debug_callback(debug_function_type cb) {
	std::cout << "Register cb\n";
	solver_debug_cb = cb;
}

void solver_debug(std::string const& message, SolverField const& solverfield, Minefield const& minefield) {
	if (solver_debug_cb) {
		solver_debug_cb(message, solverfield, minefield);
	}
}