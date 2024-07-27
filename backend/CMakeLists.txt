cmake_minimum_required(VERSION 3.25)
project(cs2-radar)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# add all files in src to the project
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
include_directories("src")

add_executable(cs2-radar ${SOURCES})
