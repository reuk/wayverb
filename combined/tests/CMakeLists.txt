cmake_minimum_required(VERSION 3.0)

project(combined_tests)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/submodules/cereal/include
    #"/usr/local/include/"
)
#link_directories(/usr/local/lib)

set(name combined_tests)
file(GLOB sources "*.cpp")

add_definitions(-DSCRATCH_PATH="${CMAKE_BINARY_DIR}")

add_executable(${name} ${sources})

target_link_libraries(${name} combined gtest)

add_test(NAME ${name} COMMAND ${name})
