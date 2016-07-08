cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")

set(name compressed-waveguide)

add_library(${name} STATIC ${sources} ${heeaders})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    "/usr/local/include/"
    ${CMAKE_SOURCE_DIR}/include
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
target_link_libraries(${name} waveguide assimp ${samplerate_lib} ${fftw_library} glog common)
