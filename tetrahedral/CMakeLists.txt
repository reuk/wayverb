cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -std=gnu++1y -msse -msse2 -msse3")

include_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

set(name tetrahedral)
file(GLOB sources "*.cpp")

add_executable(${name} ${sources})

if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(APPLE)

# if(APPLE)
#     find_library(opencl_library OpenCL)
#     mark_as_advanced(opencl_library)
#     set(frameworks ${opencl_library})
# elseif(UNIX)
#     find_library(opencl_library OpenCL PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH)
#     set(frameworks ${opencl_library})
# endif()

target_link_libraries(${name} waveguide)
