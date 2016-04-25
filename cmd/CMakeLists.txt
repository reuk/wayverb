cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/raytracer
    ${CMAKE_SOURCE_DIR}/waveguide
    ${CMAKE_SOURCE_DIR}/combined
    ${CMAKE_SOURCE_DIR}/submodules/cereal/include
    "/usr/local/include"
)

set(name wayverb)
file(GLOB sources "*.cpp")
add_executable(${name} ${sources})

if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    find_library(opencl_library OpenCL)
    mark_as_advanced(opencl_library)
    set(frameworks ${opencl_library})
elseif(UNIX)
    find_library(opencl_library OpenCL PATHS ENV LD_LIBRARY_PATH ENV OpenCL_LIBPATH)
    set(frameworks ${opencl_library})
endif()

find_library(sndfile_lib sndfile)
find_library(flac_lib flac)
find_library(ogg_lib ogg)
find_library(vorbis_lib vorbis)
find_library(vorbisenc_lib vorbisenc)

target_link_libraries(${name}
    raytracer waveguide common
    ${sndfile_lib}
    ${frameworks}
    glog
    gflags
    ${flac_lib}
    ${ogg_lib}
    ${vorbis_lib}
    ${vorbisenc_lib})
