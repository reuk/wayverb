cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Werror -std=c++14 ")

file(GLOB sources "*.cpp")

get_target_property(GLOG_PUBLIC_H glog PUBLIC_HEADER)
message(STATUS ${GLOG_PUBLIC_H})

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/submodules/glog/src
    ${CMAKE_BINARY_DIR}/submodules/glog
    ${CMAKE_SOURCE_DIR}/submodules/assimp/include
    ${CMAKE_SOURCE_DIR}/submodules/cereal/include
    "/usr/local/include/"
)

add_library(common STATIC ${sources})

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
set(libraries assimp ${fftw_library} z ${frameworks})

target_link_libraries(common ${libraries})
