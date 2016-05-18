cmake_minimum_required(VERSION 3.0)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/combined
    "/usr/local/include"
)

set(name hybrid_test)
add_executable(${name} ${name}.cpp)

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
find_library(samplerate_lib samplerate)

target_link_libraries(${name} combined raytracer waveguide common ${sndfile_lib} ${samplerate_lib} ${frameworks} gflags ${flac_lib} ${ogg_lib} ${vorbis_lib} ${vorbisenc_lib})
