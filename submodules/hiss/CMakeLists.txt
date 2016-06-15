cmake_minimum_required(VERSION 3.0)

project(HISS_CONVOLUTION VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

file(GLOB sources "**/*.cpp")

set(name hiss_convolution)

add_library(${name} STATIC ${sources})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
)

#target_link_libraries()
