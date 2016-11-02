cmake_minimum_required(VERSION 3.0)

#include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(write_hrtf main.cpp)

target_link_libraries(write_hrtf hrtf frequency_domain audio_file)

