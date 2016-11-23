cmake_minimum_required(VERSION 3.2)

add_executable(write_hrtf main.cpp)

target_link_libraries(write_hrtf hrtf frequency_domain audio_file)

