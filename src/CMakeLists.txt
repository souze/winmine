
list(APPEND IMPL_FILES 
	"solver/solver.cpp" 
	"solver/solver_helpers.cpp"
	"model/minefield.cpp"
	"control/controller.cpp"
	"lib/util.cpp")

add_executable (winmine      
	"winmine.cpp"
	"view/gui.cpp"
	${IMPL_FILES})
add_executable (winmine_test 
	"solver/test_solver.cpp"
	${IMPL_FILES})

find_package(unofficial-nana CONFIG REQUIRED)
target_link_libraries(winmine PRIVATE unofficial::nana::nana)

# To find and use catch
find_path(CATCH_INCLUDE_DIR NAMES catch.hpp PATH_SUFFIXES catch2)
include_directories(${CATCH_INCLUDE_DIR})

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(winmine_test PRIVATE Catch2::Catch2)
