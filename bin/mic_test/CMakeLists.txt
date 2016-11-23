cmake_minimum_required(VERSION 3.3)

set(name mic_offset_rotate)

add_executable(${name} ${name}.cpp)
target_link_libraries(${name} box waveguide raytracer)
