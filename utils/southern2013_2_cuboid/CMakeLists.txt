cmake_minimum_required(VERSION 3.0)

set(name southern2013_2_cuboid)
file(GLOB_RECURSE sources "*.cpp")
add_executable(${name} ${sources})

target_link_libraries(${name} waveguide raytracer common gflags)
