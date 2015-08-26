cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -std=c++1y -msse -msse2 -msse3")

file(GLOB sources "*.cpp")

add_library(waveguide STATIC ${sources})

if(APPLE)
    find_library(opencl_library OpenCL)
    mark_as_advanced(opencl_library)
    set(frameworks ${opencl_library})
elseif(UNIX)
    find_library(opencl_library OpenCL PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH)
    set(frameworks ${opencl_library})
endif()

if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(APPLE)
find_library(assimp_library assimp)

target_link_libraries(waveguide ${assimp_library} z ${frameworks})
