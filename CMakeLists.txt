cmake_minimum_required(VERSION 3.28)
project(game_of_life_mpi)

set(CMAKE_CXX_STANDARD 17)

add_executable(game_of_life_mpi main.cpp)

# Add MPI Package to Project
find_package(MPI REQUIRED)
# Add libraries for code completion and compiling
target_link_libraries(game_of_life_mpi PUBLIC MPI::MPI_CXX)