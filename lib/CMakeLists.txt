cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -std=c++1y")

file(GLOB sources "*.cpp")

add_library(waveguide STATIC ${sources})

find_library(opencl_library OpenCL)
mark_as_advanced(opencl_library)
set(frameworks ${opencl_library})

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(assimp_library assimp)

target_link_libraries(waveguide ${assimp_library} z ${frameworks})
