cmake_minimum_required(VERSION 3.2)

file(GLOB_RECURSE public_headers "include/*.h")

add_library(hrtf INTERFACE)

target_include_directories(hrtf
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
