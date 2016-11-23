cmake_minimum_required(VERSION 3.3)

set(name siltanen2013)
file(GLOB_RECURSE sources "*.cpp")
add_executable(${name} ${sources})

target_link_libraries(${name} box combined)
