cmake_minimum_required(VERSION 3.0)

project(tests)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/gtest/include
    "/usr/local/include/"
)
link_directories(/usr/local/lib)

set(name tests)
file(GLOB sources "*.cpp")

add_definitions(${test_file_flag})

add_executable(${name} ${sources})

find_library(sndfile_lib sndfile)

target_link_libraries(${name} waveguide ${sndfile_lib} gtest)

add_test(waveguide ${name})
