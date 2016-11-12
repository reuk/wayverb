cmake_minimum_required(VERSION 3.0)

set(name diffuse_decay)
file(GLOB_RECURSE sources "*.cpp")
add_executable(${name} ${sources})

target_link_libraries(${name} raytracer)
