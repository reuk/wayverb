cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_BINARY_DIR}/include)

set(name southern2011)
file(GLOB_RECURSE sources "*.cpp")
add_executable(${name} ${sources})

target_link_libraries(${name} waveguide raytracer common)
