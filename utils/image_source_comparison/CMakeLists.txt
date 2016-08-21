cmake_minimum_required(VERSION 3.0)

set(name image_source_comparison)
add_executable(${name} ${name}.cpp)

target_link_libraries(${name} waveguide common gflags)
