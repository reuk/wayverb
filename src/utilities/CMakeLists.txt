cmake_minimum_required(VERSION 3.3)

project(COMMON VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(utilities STATIC ${sources} ${public_headers} ${private_headers})

target_include_directories(utilities
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
)

add_subdirectory(tests)
