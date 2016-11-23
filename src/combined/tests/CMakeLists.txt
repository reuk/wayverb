cmake_minimum_required(VERSION 3.3)

project(combined_tests)

include_directories(${DEPENDENCY_INSTALL_PREFIX}/include)

set(name combined_tests)
file(GLOB sources "*.cpp")

add_definitions(-DSCRATCH_PATH="${CMAKE_BINARY_DIR}")

add_executable(combined_tests ${sources})

target_link_libraries(combined_tests combined gtest)

add_test(NAME combined_tests COMMAND combined_tests)
