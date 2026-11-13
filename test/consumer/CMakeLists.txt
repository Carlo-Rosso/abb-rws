# Checks that the installed abb_rws CMake package can be found and linked by a
# downstream project (run with `pixi run test-consumer`).
cmake_minimum_required(VERSION 3.16)
project(abb_rws_consumer LANGUAGES CXX)

find_package(abb_rws REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE abb_rws::abb_rws)
