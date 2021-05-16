
add_executable (winmine "winmine.cpp" "winmine.h")

find_package(unofficial-nana CONFIG REQUIRED)
target_link_libraries(winmine PRIVATE unofficial::nana::nana)
