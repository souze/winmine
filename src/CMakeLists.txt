
add_executable (winmine      "winmine.cpp"            "solver/solver.cpp" "solver/solver_helpers.cpp")
add_executable (winmine_test_gui "solver/test_solver_gui.cpp" "solver/solver.cpp" "solver/solver_helpers.cpp")
add_executable (winmine_test "solver/test_solver.cpp" "solver/solver.cpp" "solver/solver_helpers.cpp")

find_package(unofficial-nana CONFIG REQUIRED)
target_link_libraries(winmine PRIVATE unofficial::nana::nana)
target_link_libraries(winmine_test_gui PRIVATE unofficial::nana::nana)

# To find and use catch
find_path(CATCH_INCLUDE_DIR NAMES catch.hpp PATH_SUFFIXES catch2)
include_directories(${CATCH_INCLUDE_DIR})

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(winmine_test PRIVATE Catch2::Catch2)
