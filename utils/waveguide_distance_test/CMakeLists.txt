cmake_minimum_required(VERSION 3.0)

set(name waveguide_distance_test)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide common gflags)
