cmake_minimum_required(VERSION 3.2)

include_directories(${DEPENDENCY_INSTALL_PREFIX}/include)

file(GLOB sources "*.cpp")
add_executable(utilities_tests ${sources})
target_link_libraries(utilities_tests utilities gtest)
add_test(NAME utilities_tests COMMAND utilities_tests)
