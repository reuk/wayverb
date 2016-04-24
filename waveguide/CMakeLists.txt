cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

file(GLOB sources "*.cpp")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/submodules/glog/src
    ${CMAKE_BINARY_DIR}/submodules/glog
    "/usr/local/include/"
)

add_library(waveguide STATIC ${sources})

if(APPLE)
    find_library(opencl_library OpenCL)
    mark_as_advanced(opencl_library)
    set(frameworks ${opencl_library})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
elseif(UNIX)
    find_library(opencl_library OpenCL PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH)
    set(frameworks ${opencl_library})
endif()

find_library(fftw_library fftw3f)
find_library(samplerate_lib samplerate)
set(libraries
    assimp
    ${samplerate_lib}
    ${fftw_library}
    glog
    z
    common)

target_link_libraries(waveguide ${libraries})
