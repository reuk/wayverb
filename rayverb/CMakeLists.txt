cmake_minimum_required(VERSION 3.0)

project(rayverb)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -std=c++1y")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_library(rayverb STATIC helpers.cpp rayverb.cpp filters.cpp kernel.cpp hrtf.cpp)

find_library(opencl_library OpenCL)
mark_as_advanced(opencl_library)
set(frameworks ${opencl_library})

if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(APPLE)

find_library(assimp_library assimp)
find_library(fftw3f_library fftw3f)

target_link_libraries(${name} rayverb ${assimp_library} z ${fftw3f_library} ${frameworks})
