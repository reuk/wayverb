cmake_minimum_required(VERSION 3.0)

project(combined_tests)

include_directories(${CMAKE_BINARY_DIR}/include)

set(name combined_tests)
file(GLOB sources "*.cpp")

add_definitions(-DSCRATCH_PATH="${CMAKE_BINARY_DIR}")

add_executable(combined_tests ${sources})

target_link_libraries(combined_tests combined gtest)

add_test(NAME combined_tests COMMAND combined_tests)
