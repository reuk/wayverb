cmake_minimum_required(VERSION 3.0)

project(WAVEGUIDE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++14 ")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror ")
endif()

file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE headers "include/*.h")

set(name waveguide)

add_library(${name} STATIC ${sources} ${headers})

target_include_directories(${name}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    src
    ${WAYVERB_SOURCE_DIR}/include
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
target_link_libraries(${name} assimp wayverb-data ${samplerate_lib} ${fftw_library} common)

add_subdirectory(compressed)
