cmake_minimum_required(VERSION 3.0)

project(WAVEGUIDE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

file(GLOB sources "src/*.cpp")

add_library(waveguide STATIC ${sources})

target_include_directories(waveguide
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
    ${WAYVERB_SOURCE_DIR}/include
    ${WAYVERB_SOURCE_DIR}/submodules/glog/src
    ${WAYVERB_BINARY_DIR}/submodules/glog
    ${WAYVERB_SOURCE_DIR}/submodules/cereal/include
    "/usr/local/include/"
)

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
target_link_libraries(waveguide assimp ${samplerate_lib} ${fftw_library} glog z common)
