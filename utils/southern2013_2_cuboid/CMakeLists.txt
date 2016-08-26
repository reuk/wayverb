cmake_minimum_required(VERSION 3.0)

set(name southern2013_2_cuboid)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide raytracer common gflags)
