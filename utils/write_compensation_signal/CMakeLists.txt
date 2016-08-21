cmake_minimum_required(VERSION 3.0)

project(WRITE_COMPENSATION_SIGNAL VERSION 0.1 LANGUAGES CXX)

add_executable(write_compensation_signal write_compensation_signal.cpp)

target_link_libraries(
    write_compensation_signal
    waveguide
    compressed-waveguide
    common)
