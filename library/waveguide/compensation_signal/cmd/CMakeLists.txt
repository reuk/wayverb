cmake_minimum_required(VERSION 3.0)

include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(write_compensation_signal main.cpp)

target_link_libraries(write_compensation_signal compensation_signal common)
