cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")

set(name compressed-waveguide)

add_library(${name} STATIC ${sources} ${heeaders})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${name} waveguide)
