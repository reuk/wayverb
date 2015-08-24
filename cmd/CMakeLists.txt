cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -std=c++1y")

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

set(name pwaveguide)
file(GLOB sources "*.cpp")

add_executable(${name} ${sources})

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(sndfile_library sndfile)

find_library(opencl_library OpenCL)
mark_as_advanced(opencl_library)
set(frameworks ${opencl_library})

target_link_libraries(${name} waveguide ${sndfile_library} ${frameworks})
