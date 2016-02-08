cmake_minimum_required(VERSION 3.0)

file(GLOB sources "*.cpp")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/rayverb
    ${CMAKE_SOURCE_DIR}/waveguide
    "/usr/local/include/"
)

set(name combined)

add_library(${name} STATIC ${sources})

set(libraries rayverb waveguide common)

target_link_libraries(${name} ${libraries})
