cmake_minimum_required(VERSION 3.3)

set(name boundary_test)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide)
