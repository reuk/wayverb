cmake_minimum_required(VERSION 3.0)

file(GLOB sources "*.cpp")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/raytracer
    ${CMAKE_SOURCE_DIR}/waveguide
    "/usr/local/include/"
)

set(name combined)

#add_library(${name} STATIC ${sources})
#target_link_libraries(${name} raytracer waveguide common)
