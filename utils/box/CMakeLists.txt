cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(box STATIC ${sources} ${public_headers} ${private_headers})

target_include_directories(box
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(box combined waveguide raytracer common)
