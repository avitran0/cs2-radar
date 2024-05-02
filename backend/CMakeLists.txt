cmake_minimum_required(VERSION 3.25)
project(cs2-radar)

set(CMAKE_CXX_STANDARD 20)

# add all files in src to the project
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
include_directories("src")

add_executable(cs2-radar ${SOURCES})
