cmake_minimum_required(VERSION 3.0)

project(COMBINED VERSION 0.1 LANGUAGES CXX)

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE public_headers "include/*.h")
file(GLOB_RECURSE private_headers "src/*.h")

add_library(combined STATIC ${sources} ${public_headers} ${private_headers})

add_dependencies(combined modern_gl_utils_external)

target_include_directories(combined
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(combined waveguide raytracer)

add_subdirectory(tests)
