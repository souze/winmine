# CMakeList.txt : CMake project for winmine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(WinMine)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory("src")

