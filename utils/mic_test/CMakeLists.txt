cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_BINARY_DIR}/include)

set(name mic_offset_rotate)

add_executable(${name} ${name}.cpp)
target_link_libraries(${name} waveguide raytracer common)
