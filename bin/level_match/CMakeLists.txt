cmake_minimum_required(VERSION 3.2)

set(name level_match)
file(GLOB_RECURSE sources "*.cpp")
add_executable(${name} ${sources})

target_link_libraries(${name} box combined)
