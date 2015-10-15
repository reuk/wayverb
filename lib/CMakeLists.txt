cmake_minimum_required(VERSION 3.0)

file(GLOB sources "*.cpp")

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

find_library(assimp_library assimp)
find_library(fftw_library fftw3f)
find_package(rtaudiocommon REQUIRED)
set(libraries rtaudiocommon ${assimp_library} ${fftw_library} z)

target_link_libraries(waveguide ${libraries})
