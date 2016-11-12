cmake_minimum_required(VERSION 3.0)

set(name fitted_boundary)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide)
