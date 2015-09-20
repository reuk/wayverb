cmake_minimum_required(VERSION 3.0)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

set(name tetrahedral)
file(GLOB sources "*.cpp")

add_executable(${name} ${sources})

if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(APPLE)

target_link_libraries(${name} waveguide gflags)
