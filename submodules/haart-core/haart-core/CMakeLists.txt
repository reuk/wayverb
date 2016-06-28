cmake_minimum_required(VERSION 3.0)

project(HAART_CORE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h" "*.hpp")

set(name haart-core)

add_library(${name} STATIC ${sources} ${headers})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
)
