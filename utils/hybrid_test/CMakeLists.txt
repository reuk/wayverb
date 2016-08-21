cmake_minimum_required(VERSION 3.0)

set(name hybrid_test)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} combined raytracer waveguide common gflags)
