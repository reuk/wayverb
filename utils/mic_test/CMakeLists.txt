cmake_minimum_required(VERSION 3.0)

set(name_1 mic_offset_rotate)

add_executable(${name_1} ${name_1}.cpp)
target_link_libraries(${name_1} waveguide common gflags)
